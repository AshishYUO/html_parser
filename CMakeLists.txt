cmake_minimum_required(VERSION 3.13)

project(html_parser)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "-O2")

add_executable(html_parser main.cpp src/html_parser.cpp src/dom_element.cpp)
