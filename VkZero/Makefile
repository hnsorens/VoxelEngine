# Makefile for VoxelEngine

all:
	g++ -fPIC -std=c++23 -DGLFW_INCLUDE_VULKAN -o VkZero.so src/*.cpp -Iinclude -lvulkan -lglfw -L. -shared 

clean:
	rm -rf bin/* shaders/*.spv

run:
	./bin/voxels 
