cmake_minimum_required(VERSION 2.10)
project(linearV1)

add_compile_options(-fopenmp)

add_executable(linearV1 linear_eq1.cpp)
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OpenMP_CXX_LIBRARIES})
