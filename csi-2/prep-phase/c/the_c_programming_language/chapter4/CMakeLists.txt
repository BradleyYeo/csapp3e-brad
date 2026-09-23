cmake_minimum_required(VERSION 3.10)

# Set the project name and version
project(Test VERSION 1.0)

add_compile_options(-Wall -Wuninitialized)

# Specify the C Standard
set(CMAKE_C_STANDARD 99)

# Add executable
add_executable(test test.c)

