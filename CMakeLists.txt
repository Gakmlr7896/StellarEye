cmake_minimum_required(VERSION 3.10)
project(StellarEye)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find Torch
set(CMAKE_PREFIX_PATH "/usr/local/libtorch")
find_package(Torch REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})

add_executable(test src/test.cpp)
add_executable(test2 src/test2.cpp)

target_link_libraries(test ${OpenCV_LIBS} ${TORCH_LIBRARIES})
target_link_libraries(test2 ${OpenCV_LIBS} ${TORCH_LIBRARIES})
