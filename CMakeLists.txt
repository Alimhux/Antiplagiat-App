cmake_minimum_required(VERSION 3.20)
project(antiplagiat_app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(api-gateway)
add_subdirectory(file-storing-service)
add_subdirectory(file-analysis-service)