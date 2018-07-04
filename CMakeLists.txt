cmake_minimum_required(VERSION 3.8)
project(finder_pattern_detector)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)
set(SOURCE_FILES main.cpp FinderPatternDetector.h FinderPatternDetector.cpp)
add_executable(finder_pattern_detector ${SOURCE_FILES})
include_directories(${OpenCV_INCLUDE_DIRS})
target_link_libraries( finder_pattern_detector ${OpenCV_LIBS} )