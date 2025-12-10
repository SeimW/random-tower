cmake_minimum_required(VERSION 4.0)
project(RandomTower)

set(CMAKE_CXX_STANDARD 20)

#GLAD
add_library(glad libs/glad/glad.c)
target_include_directories(glad PUBLIC libs/glad/include)

#GLFW
add_subdirectory(libs/glfw)

#Executable
add_executable(RandomTower main.cpp
        Level.h
)
target_include_directories(RandomTower PUBLIC libs/glad/include libs/glfw/include)
target_link_libraries(RandomTower glad glfw opengl32)

set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
set_target_properties(RandomTower PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}"
)
