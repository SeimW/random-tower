cmake_minimum_required(VERSION 4.0)
project(RandomTower)

set(CMAKE_CXX_STANDARD 20)

#GLAD
add_library(glad libs/glad/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

#GLFW
add_subdirectory(libs/glfw)

#Executable
add_executable(RandomTower main.cpp)
target_include_directories(RandomTower PUBLIC libs/glad/include libs/glfw/include)
target_link_libraries(RandomTower glad glfw opengl32)
