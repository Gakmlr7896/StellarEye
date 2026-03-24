cmake_minimum_required(VERSION 3.10)
project(FaceDetectionTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(test src/test.cpp)

target_link_libraries(test ${OpenCV_LIBS})
