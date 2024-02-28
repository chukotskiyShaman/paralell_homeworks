cmake_minimum_required(VERSION 2.10)
project(matrix)

add_executable(matrix first.cpp)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})