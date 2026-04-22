cmake_minimum_required(VERSION 3.10)
project(StellarEye)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenCV
find_package(OpenCV REQUIRED)

# Find Torch (Manual configuration to avoid CUDA compiler issues)
set(TORCH_INSTALL_DIR "/usr/local/libtorch")
set(TORCH_INCLUDE_DIRS 
    "${TORCH_INSTALL_DIR}/include" 
    "${TORCH_INSTALL_DIR}/include/torch/csrc/api/include"
)
# Add Miniconda lib path for libgfortran
set(LIBGFORTRAN_PATH "/home/phar/miniconda3/pkgs/libgfortran5-15.2.0-hc633d37_7/lib/libgfortran.so.5")
link_directories("${TORCH_INSTALL_DIR}/lib")
set(TORCH_LIBRARIES torch torch_cpu c10 "${LIBGFORTRAN_PATH}")

include_directories(${OpenCV_INCLUDE_DIRS} ${TORCH_INCLUDE_DIRS})

add_executable(test src/TestModels/TestDistanceModel.cpp)

target_link_libraries(test ${OpenCV_LIBS} ${TORCH_LIBRARIES})
# Ensure rpath is set so the executable can find LibTorch and libgfortran shared libraries at runtime
set_target_properties(test PROPERTIES INSTALL_RPATH "${TORCH_INSTALL_DIR}/lib;/home/phar/miniconda3/pkgs/libgfortran5-15.2.0-hc633d37_7/lib")
set_target_properties(test PROPERTIES BUILD_WITH_INSTALL_RPATH TRUE)
