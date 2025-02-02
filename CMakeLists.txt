cmake_minimum_required(VERSION 3.10)
project(VoxelEngine)

find_package(OpenMP REQUIRED)

# Specify the directories containing the header files
include_directories(${CMAKE_SOURCE_DIR}/Includes)

set(CMAKE_BUILD_TYPE Debug)


# Specify the directories containing the .so library files
link_directories(${CMAKE_SOURCE_DIR}/Lib)

# Define the executable
add_executable(VoxelEngine
            main.cpp 
)

# Link the executable to the local GLFW and Vulkan libraries
target_link_libraries(VoxelEngine glfw vulkan)