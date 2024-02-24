cmake_minimum_required(VERSION 2.10)
project(matrix)

add_executable(matrix first.cpp)

find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})