cmake_minimum_required(VERSION 3.5)

project(nm2surf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV 4.5 REQUIRED)

add_executable(nm2surf main.cpp
    util_functions.h util_functions.cpp
    math_functions.h math_functions.cpp)


target_link_libraries(nm2surf ${OpenCV_LIBS} pthread tbb)

include(GNUInstallDirs)
install(TARGETS nm2surf
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
