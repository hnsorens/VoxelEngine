# Makefile for VoxelEngine

all:
	glslc --target-spv=spv1.4 -fshader-stage=rgen shaders/rgen.glsl -o bin/rgen.spv
	glslc --target-spv=spv1.4 -fshader-stage=rmiss shaders/rmiss.glsl -o bin/rmiss.spv
	glslc --target-spv=spv1.4 -fshader-stage=frag shaders/frag.glsl -o bin/frag.spv
	glslc --target-spv=spv1.4 -fshader-stage=vert shaders/vert.glsl -o bin/vert.spv
	g++ -std=c++23 -DGLFW_INCLUDE_VULKAN src/*.cpp src/VkZero/*.cpp -Iinclude -o bin/voxels -lvulkan -lglfw -lglm

clean:
	rm -rf bin/* shaders/*.spv

run: all
	./bin/voxels 