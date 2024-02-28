cmake_minimum_required(VERSION 3.10)
project(integral)

add_executable(integral second.cpp)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})