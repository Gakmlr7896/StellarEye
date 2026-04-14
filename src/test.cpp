// This is a test cpp file for educational uses , Its ""NOT"" a part of the project and will be deleted

#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    // Try to find the video directory
    std::string videoDir = "./Data/TestVidData";
    if (!fs::exists(videoDir)) {
        videoDir = "../Data/TestVidData";
    }
    
    // Load the Haar cascade for face detection
    cv::CascadeClassifier faceCascade;
    std::string cascadePath = "/home/phar/miniconda3/envs/DFM/share/opencv4/haarcascades/haarcascade_frontalface_default.xml";
    if (!faceCascade.load(cascadePath)) {
        std::cerr << "Error: Could not load Haar cascade at: " << cascadePath << std::endl;
        return -1;
    }

    if (!fs::exists(videoDir)) {
        std::cerr << "Error: Directory not found: " << videoDir << std::endl;
        return -1;
    }

    std::cout << "Starting video processing from: " << videoDir << std::endl;
    std::cout << "Press 'q' to skip current video, 'ESC' to exit all." << std::endl;

    const std::string windowName = "Face Detection Test";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    for (const auto& entry : fs::directory_iterator(videoDir)) {
        if (entry.path().extension() == ".mp4") {
            std::string videoPath = entry.path().string();
            std::cout << "Processing: " << videoPath << std::endl;

            cv::VideoCapture cap(videoPath);
            if (!cap.isOpened()) {
                std::cerr << "Error: Could not open video: " << videoPath << std::endl;
                continue;
            }

            cv::Mat frame, gray;
            while (true) {
                cap >> frame;
                if (frame.empty()) break;

                cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
                cv::equalizeHist(gray, gray);

                std::vector<cv::Rect> faces;
                faceCascade.detectMultiScale(gray, faces, 1.1, 10, 0, cv::Size(30, 30));

                for (const auto& face : faces) {
                    cv::rectangle(frame, face, cv::Scalar(0, 0, 255), 2);
                }

                // Resize the frame for display if it's too large
                cv::Mat displayFrame;
                double scale = 0.5; // Scale factor (0.5 = 50% size)
                cv::resize(frame, displayFrame, cv::Size(), scale, scale);

                cv::imshow(windowName, displayFrame);
                
                char key = (char)cv::waitKey(30); // 30ms delay to approximate real-time playback
                if (key == 'q') break; // Skip to next video
                if (key == 27) return 0; // ESC key to exit program
            }
            cap.release();
        }
    }

    cv::destroyAllWindows();
    std::cout << "Finished processing all videos." << std::endl;
    return 0;
}

