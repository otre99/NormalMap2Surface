cmake_minimum_required(VERSION 3.5)

project(DisplaySurf VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 COMPONENTS Core)
find_package(Qt6 COMPONENTS Gui)
find_package(Qt6 COMPONENTS Widgets)
# find_package(Qt6 COMPONENTS Graphs)
find_package(Qt6 COMPONENTS DataVisualization)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

qt_add_executable(DisplaySurf ${PROJECT_SOURCES}
    surfacedata.h surfacedata.cpp)


target_link_libraries(DisplaySurf PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Widgets
    Qt::DataVisualization
)


include(GNUInstallDirs)
install(TARGETS DisplaySurf
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(DisplaySurf)
endif()
