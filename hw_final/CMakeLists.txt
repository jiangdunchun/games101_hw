cmake_minimum_required(VERSION 2.8)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 14)
project(FFT_water_simulation)
if("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Windows")
    include_directories(${PROJECT_SOURCE_DIR}/include/)
    link_directories(${PROJECT_SOURCE_DIR}/lib/)
elseif("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Linux")
    find_package(glfw3 REQUIRED)
endif()

file(GLOB project_file glad.c main.cpp)
add_executable(${PROJECT_NAME} ${project_file})

if("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Windows")
    include_directories(${PROJECT_SOURCE_DIR}/include/)
    link_directories(${PROJECT_SOURCE_DIR}/lib/)
    target_link_libraries(${PROJECT_NAME} glfw)
elseif("!${CMAKE_SYSTEM_NAME}" STREQUAL "!Linux")
    target_link_libraries(${PROJECT_NAME}
    -lglfw
    -lGL
    -lm
    -lXrandr
    -lXi
    -lX11
    -lXxf86vm
    -lpthread
    -ldl
    -lXinerama
    -lXcursor)
endif()

