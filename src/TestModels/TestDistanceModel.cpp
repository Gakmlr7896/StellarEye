#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <torch/torch.h>
#include <chrono>
#include <algorithm>

struct IrisModel : torch::nn::Module {
    	IrisModel(int in_features, int h1, int h2, int out_features) {
        	fc1 = register_module("fc1", torch::nn::Linear(in_features, h1));
        	fc2 = register_module("fc2", torch::nn::Linear(h1, h2));
        	fc3 = register_module("fc3", torch::nn::Linear(h2, out_features));
    	}

    	torch::Tensor forward(torch::Tensor x) {
        	x = torch::relu(fc1->forward(x));
        	x = torch::relu(fc2->forward(x));
        	x = fc3->forward(x);
        	return x;
    	}

    	torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
};

// Simple CSV parser for Iris data
std::pair<torch::Tensor, torch::Tensor> load_iris_data(const std::string& path) {
    	std::ifstream file(path);
    	if (!file.is_open()) {
        	throw std::runtime_error("Could not open file: " + path);
    	}

    	std::string line;
    	std::vector<float> features;
    	std::vector<long> labels;

    	while (std::getline(file, line)) {
        	if (line.empty()) continue;
        	std::stringstream ss(line);
        	std::string val;
        
		try {
            		for (int i = 0; i < 4; ++i) {
                		if (!std::getline(ss, val, ',')) break;
                		features.push_back(std::stof(val));
            		}
            		if (!std::getline(ss, val, ',')) continue;
            
            		// Trim potential whitespace/newline from label
            		val.erase(val.find_last_not_of(" \n\r\t") + 1);
            		val.erase(0, val.find_first_not_of(" \n\r\t"));

            		if (val == "Iris-setosa") labels.push_back(0);
            		else if (val == "Iris-versicolor") labels.push_back(1);
            		else if (val == "Iris-virginica") labels.push_back(2);
            		else {
                		// If label doesn't match, we might have an extra comma or different format
                		// In some iris.data versions there are empty lines or headers
                		for (int i = 0; i < 4; ++i) features.pop_back(); 
        		}
        	} 
		catch (...) {
            		// Skip lines that don't match the format
            		continue;
        	}
    	}

    	if (labels.empty()) {
        	throw std::runtime_error("No data loaded from: " + path);
    	}

    	auto features_tensor = torch::from_blob(features.data(), {(long)labels.size(), 4}).clone();
    	auto labels_tensor = torch::from_blob(labels.data(), {(long)labels.size()}, torch::kLong).clone();

    	return {features_tensor, labels_tensor};
}

int main() {
    	try {
        	const int in_features = 4;
        	int h1 = 8;
        	int h2 = 9;
        	const int out_features = 3;

        	auto model = std::make_shared<IrisModel>(in_features, h1, h2, out_features);
        
        	std::string data_path = "Data/testdata/iris.data";
        	std::ifstream f(data_path);
        	if (!f.good()) {
        		data_path = "../Data/testdata/iris.data";
        	}
        	f.close();

        	auto data = load_iris_data(data_path);
        	auto features = data.first;
        	auto labels = data.second;

        	std::cout << "Loaded " << labels.size(0) << " samples." << std::endl;

        	torch::optim::Adam optimizer(model->parameters(), torch::optim::AdamOptions(0.01));

        	std::cout << "Starting training..." << std::endl;
        	for (int epoch = 1; epoch <= 200; ++epoch) {
            		optimizer.zero_grad();
            		auto output = model->forward(features);
            		auto loss = torch::nn::functional::cross_entropy(output, labels);
            		loss.backward();
            		optimizer.step();

            		if (epoch % 20 == 0) {
                		std::cout << "Epoch: " << epoch << " | Loss: " << loss.item<float>() << std::endl;
            		}
        	}

        	// Final accuracy check
        	model->eval();
        	auto final_output = model->forward(features);
        	auto predicted = final_output.argmax(1);
        	auto correct = predicted.eq(labels).sum().item<int>();
       		std::cout << "Final Accuracy: " << (float)correct / labels.size(0) * 100 << "%" << std::endl;

        	std::cout << "Training complete!" << std::endl;

    	}
    	catch (const std::exception& e) {
        	std::cerr << "Error: " << e.what() << std::endl;
        	return 1;
    	}
	
    	return 0;
}
