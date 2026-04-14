// ============================================================
//  SalaryPredictor — Linear Regression in pure C++
//  QOctopus / StellarEye style: no ML libs, no Python.
//  Features: experience_years, skills_count, certifications
//            + one-hot: education_level, company_size,
//                       remote_work, industry, location
//  Target: salary
// ============================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iomanip>

// ─── helpers ────────────────────────────────────────────────

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n\"");
    size_t b = s.find_last_not_of(" \t\r\n\"");
    return (a == std::string::npos) ? "" : s.substr(a, b - a + 1);
}

// Split a CSV line respecting quoted fields
static std::vector<std::string> split_csv(const std::string& line) {
    std::vector<std::string> tokens;
    std::string cur;
    bool in_quotes = false;
    for (char c : line) {
        if (c == '"')  { in_quotes = !in_quotes; }
        else if (c == ',' && !in_quotes) { tokens.push_back(trim(cur)); cur.clear(); }
        else cur += c;
    }
    tokens.push_back(trim(cur));
    return tokens;
}

// ─── one-hot encoder ────────────────────────────────────────

struct OneHotEncoder {
    std::string           col_name;
    std::vector<std::string> categories;   // sorted at fit time

    // fit: collect unique values from training column
    void fit(const std::vector<std::string>& col) {
        std::vector<std::string> uniq = col;
        std::sort(uniq.begin(), uniq.end());
        uniq.erase(std::unique(uniq.begin(), uniq.end()), uniq.end());
        categories = uniq;
    }

    // transform: drop-first one-hot (avoids multicollinearity)
    std::vector<double> encode(const std::string& val) const {
        std::vector<double> vec(categories.size() - 1, 0.0);
        for (size_t i = 1; i < categories.size(); ++i)
            if (categories[i] == val) { vec[i - 1] = 1.0; break; }
        return vec;
    }

    size_t out_dim() const { return categories.size() > 1 ? categories.size() - 1 : 0; }
};

// ─── dataset row ────────────────────────────────────────────

struct Row {
    // raw
    double experience_years;
    double skills_count;
    double certifications;
    std::string education_level;
    std::string company_size;
    std::string remote_work;
    std::string industry;
    std::string location;
    double salary;           // target
};

// ─── CSV loader ─────────────────────────────────────────────

static std::vector<Row> load_csv(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) {
        std::cerr << "[ERROR] Cannot open file: " << path << "\n";
        std::exit(1);
    }

    std::vector<Row> rows;
    std::string line;

    // header
    std::getline(f, line);
    auto header = split_csv(line);
    // find column indices dynamically
    std::map<std::string,int> col_idx;
    for (int i = 0; i < (int)header.size(); ++i)
        col_idx[header[i]] = i;

    auto need = [&](const std::string& n) {
        if (col_idx.find(n) == col_idx.end()) {
            std::cerr << "[ERROR] Missing column: " << n << "\n"; std::exit(1);
        }
        return col_idx[n];
    };

    int i_exp  = need("experience_years");
    int i_ski  = need("skills_count");
    int i_cer  = need("certifications");
    int i_edu  = need("education_level");
    int i_cmp  = need("company_size");
    int i_rem  = need("remote_work");
    int i_ind  = need("industry");
    int i_loc  = need("location");
    int i_sal  = need("salary");

    while (std::getline(f, line)) {
        if (line.empty()) continue;
        auto tok = split_csv(line);
        if ((int)tok.size() <= std::max({i_exp,i_ski,i_cer,i_edu,i_cmp,i_rem,i_ind,i_loc,i_sal}))
            continue;
        Row r;
        try {
            r.experience_years = std::stod(tok[i_exp]);
            r.skills_count     = std::stod(tok[i_ski]);
            r.certifications   = std::stod(tok[i_cer]);
            r.salary           = std::stod(tok[i_sal]);
        } catch (...) { continue; }
        r.education_level = tok[i_edu];
        r.company_size    = tok[i_cmp];
        r.remote_work     = tok[i_rem];
        r.industry        = tok[i_ind];
        r.location        = tok[i_loc];
        rows.push_back(r);
    }
    return rows;
}

// ─── feature builder ────────────────────────────────────────

struct FeatureBuilder {
    OneHotEncoder enc_edu, enc_cmp, enc_rem, enc_ind, enc_loc;
    // normalisation stats for numeric cols (z-score)
    double mean_exp, std_exp;
    double mean_ski, std_ski;
    double mean_cer, std_cer;
    double mean_sal, std_sal;

    void fit(const std::vector<Row>& rows) {
        // one-hot
        std::vector<std::string> edu, cmp, rem, ind, loc;
        for (auto& r : rows) {
            edu.push_back(r.education_level);
            cmp.push_back(r.company_size);
            rem.push_back(r.remote_work);
            ind.push_back(r.industry);
            loc.push_back(r.location);
        }
        enc_edu.fit(edu); enc_cmp.fit(cmp); enc_rem.fit(rem);
        enc_ind.fit(ind); enc_loc.fit(loc);

        // numeric means
        auto stats = [&](std::vector<double> v) -> std::pair<double,double> {
            double m = std::accumulate(v.begin(),v.end(),0.0) / v.size();
            double s = 0;
            for (auto x : v) s += (x-m)*(x-m);
            s = std::sqrt(s / v.size());
            return {m, s < 1e-9 ? 1.0 : s};
        };

        std::vector<double> vexp, vski, vcer, vsal;
        for (auto& r : rows) {
            vexp.push_back(r.experience_years);
            vski.push_back(r.skills_count);
            vcer.push_back(r.certifications);
            vsal.push_back(r.salary);
        }
        auto [me,se] = stats(vexp); mean_exp=me; std_exp=se;
        auto [ms,ss] = stats(vski); mean_ski=ms; std_ski=ss;
        auto [mc,sc] = stats(vcer); mean_cer=mc; std_cer=sc;
        auto [msa,ssa] = stats(vsal); mean_sal=msa; std_sal=ssa;
    }

    // returns feature vector (NOT including bias — added later)
    std::vector<double> transform(const Row& r) const {
        std::vector<double> x;
        x.push_back((r.experience_years - mean_exp) / std_exp);
        x.push_back((r.skills_count     - mean_ski) / std_ski);
        x.push_back((r.certifications   - mean_cer) / std_cer);
        auto app = [&](const std::vector<double>& v){ for (auto d : v) x.push_back(d); };
        app(enc_edu.encode(r.education_level));
        app(enc_cmp.encode(r.company_size));
        app(enc_rem.encode(r.remote_work));
        app(enc_ind.encode(r.industry));
        app(enc_loc.encode(r.location));
        return x;
    }

    double norm_target(double y)   const { return (y - mean_sal) / std_sal; }
    double denorm_target(double y) const { return y * std_sal + mean_sal;   }

    size_t feature_dim() const {
        return 3
            + enc_edu.out_dim() + enc_cmp.out_dim()
            + enc_rem.out_dim() + enc_ind.out_dim()
            + enc_loc.out_dim();
    }
};

// ─── linear regression (batch gradient descent) ─────────────

struct LinearRegression {
    std::vector<double> w;   // weights  (feature_dim)
    double              b;   // bias

    void init(size_t dim) {
        w.assign(dim, 0.0);
        b = 0.0;
    }

    double predict(const std::vector<double>& x) const {
        double y = b;
        for (size_t i = 0; i < w.size(); ++i) y += w[i] * x[i];
        return y;
    }

    // mini-batch SGD
    void train(const std::vector<std::vector<double>>& X,
               const std::vector<double>& Y,
               double lr, int epochs, int batch_size)
    {
        size_t n = X.size();
        std::vector<size_t> idx(n);
        std::iota(idx.begin(), idx.end(), 0);

        for (int ep = 0; ep < epochs; ++ep) {
            // shuffle
            for (size_t i = n-1; i > 0; --i) {
                size_t j = rand() % (i+1);
                std::swap(idx[i], idx[j]);
            }

            double total_loss = 0.0;
            size_t batches = 0;

            for (size_t start = 0; start < n; start += batch_size) {
                size_t end = std::min(start + (size_t)batch_size, n);
                size_t bs  = end - start;

                // accumulate gradients
                std::vector<double> grad_w(w.size(), 0.0);
                double grad_b = 0.0;

                for (size_t k = start; k < end; ++k) {
                    size_t i = idx[k];
                    double err = predict(X[i]) - Y[i];
                    total_loss += err * err;
                    grad_b += err;
                    for (size_t j = 0; j < w.size(); ++j)
                        grad_w[j] += err * X[i][j];
                }

                // update
                b -= lr * grad_b / bs;
                for (size_t j = 0; j < w.size(); ++j)
                    w[j] -= lr * grad_w[j] / bs;

                ++batches;
            }

            if ((ep+1) % 10 == 0 || ep == 0) {
                double rmse = std::sqrt(total_loss / n);
                std::cout << "  Epoch " << std::setw(3) << ep+1
                          << "  RMSE(norm): " << std::fixed << std::setprecision(5) << rmse << "\n";
            }
        }
    }
};

// ─── evaluation ─────────────────────────────────────────────

static void evaluate(LinearRegression& model,
                     const std::vector<std::vector<double>>& X,
                     const std::vector<double>& Y_norm,
                     const FeatureBuilder& fb,
                     const std::string& label)
{
    double ss_res = 0, ss_tot = 0;
    double sum_y = std::accumulate(Y_norm.begin(), Y_norm.end(), 0.0);
    double mean_y = sum_y / Y_norm.size();
    double mae = 0;

    for (size_t i = 0; i < X.size(); ++i) {
        double pred = model.predict(X[i]);
        double err  = pred - Y_norm[i];
        ss_res += err * err;
        ss_tot += (Y_norm[i] - mean_y) * (Y_norm[i] - mean_y);
        mae    += std::abs(err);
    }

    double r2   = 1.0 - ss_res / ss_tot;
    double rmse = std::sqrt(ss_res / X.size());
    double mae_val = mae / X.size();

    // de-normalise for human-readable error
    double rmse_usd = rmse * fb.std_sal;
    double mae_usd  = mae_val * fb.std_sal;

    std::cout << "\n[" << label << "]\n"
              << "  R²   = " << std::fixed << std::setprecision(4) << r2   << "\n"
              << "  RMSE = $" << std::fixed << std::setprecision(0) << rmse_usd << "\n"
              << "  MAE  = $" << std::fixed << std::setprecision(0) << mae_usd  << "\n";
}

// ─── main ───────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::string data_path = "Data/testdata/job_salary_prediction_dataset.csv";
    if (argc > 1) data_path = argv[1];

    std::cout << "Loading data from: " << data_path << "\n";
    auto rows = load_csv(data_path);
    std::cout << "Loaded " << rows.size() << " rows.\n";

    // ── train / test split (80/20) ─────────────────────────
    std::srand(42);
    std::vector<size_t> order(rows.size());
    std::iota(order.begin(), order.end(), 0);
    for (size_t i = order.size()-1; i > 0; --i) {
        size_t j = rand() % (i+1);
        std::swap(order[i], order[j]);
    }

    size_t train_n = (size_t)(rows.size() * 0.8);
    std::vector<Row> train_rows, test_rows;
    for (size_t i = 0; i < order.size(); ++i) {
        if (i < train_n) train_rows.push_back(rows[order[i]]);
        else             test_rows.push_back(rows[order[i]]);
    }
    std::cout << "Train: " << train_rows.size() << "  Test: " << test_rows.size() << "\n\n";

    // ── fit encoder & normaliser on TRAIN only ─────────────
    FeatureBuilder fb;
    fb.fit(train_rows);

    size_t feat_dim = fb.feature_dim();
    std::cout << "Feature dimension: " << feat_dim << "\n\n";

    // build matrices
    auto build_XY = [&](const std::vector<Row>& rs)
        -> std::pair<std::vector<std::vector<double>>, std::vector<double>>
    {
        std::vector<std::vector<double>> X;
        std::vector<double> Y;
        for (auto& r : rs) {
            X.push_back(fb.transform(r));
            Y.push_back(fb.norm_target(r.salary));
        }
        return {X, Y};
    };

    auto [X_train, Y_train] = build_XY(train_rows);
    auto [X_test,  Y_test ] = build_XY(test_rows);

    // ── train ──────────────────────────────────────────────
    LinearRegression model;
    model.init(feat_dim);

    double lr         = 0.01;
    int    epochs     = 100;
    int    batch_size = 256;

    std::cout << "Training  (lr=" << lr << "  epochs=" << epochs
              << "  batch=" << batch_size << ")\n";
    model.train(X_train, Y_train, lr, epochs, batch_size);

    // ── evaluate ───────────────────────────────────────────
    evaluate(model, X_train, Y_train, fb, "TRAIN");
    evaluate(model, X_test,  Y_test,  fb, "TEST ");

    // ── sample predictions ─────────────────────────────────
    std::cout << "\n--- Sample predictions (first 5 test rows) ---\n";
    std::cout << std::setw(20) << "Job Title"
              << std::setw(12) << "Actual"
              << std::setw(12) << "Predicted"
              << std::setw(10) << "Error\n";
    for (size_t i = 0; i < std::min((size_t)5, test_rows.size()); ++i) {
        double pred_norm = model.predict(X_test[i]);
        double pred_sal  = fb.denorm_target(pred_norm);
        double real_sal  = test_rows[i].salary;
        std::cout << std::setw(20) << test_rows[i].education_level
                  << std::setw(12) << (int)real_sal
                  << std::setw(12) << (int)pred_sal
                  << std::setw(9)  << (int)(pred_sal - real_sal) << "\n";
    }

    std::cout << "\nDone.\n";
    return 0;
}
