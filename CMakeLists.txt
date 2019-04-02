cmake_minimum_required(VERSION 3.5)

set(CMAKE_CXX_STANDARD 14)

project(cmake-application-example VERSION 0.0.1 LANGUAGES CXX)

add_executable(gpsToImage src/main.cpp)

target_link_libraries(gpsToImage PUBLIC )
