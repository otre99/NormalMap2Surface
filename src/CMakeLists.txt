cmake_minimum_required(VERSION 3.5)

project(nm2surf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4.5 REQUIRED)

set(SRC util_functions.h util_functions.cpp math_functions.h math_functions.cpp)

add_executable(nm2surf_cpu main_cpu.cpp ${SRC})
target_link_libraries(nm2surf_cpu ${OpenCV_LIBS} pthread tbb)


if(OpenCV_CUDA_VERSION)
    message(STATUS "OpenCV was built with CUDA support. " "CUDA_VERSION=" ${OpenCV_CUDA_VERSION})
    add_executable(nm2surf_gpu main_cuda.cpp ${SRC})
    target_link_libraries(nm2surf_gpu ${OpenCV_LIBS} pthread tbb)
else()
    message(WARNING "OpenCV was not built with CUDA support. The program nm2surf_gpu will not be built!")
endif()


