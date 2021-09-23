cmake_minimum_required(VERSION 3.10)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

project(fft_water_simulation)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assert
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(./third_party)

include_directories(${STB_INCLUDE_DIR})
include_directories(${GLM_INCLUDE_DIR})
include_directories(${GLAD_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})

add_executable(fft_water_simulation ./main.cpp)

target_link_libraries(fft_water_simulation glad glfw)

