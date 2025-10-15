# Makefile for VoxelEngine

all:
	ctags -R
	glslc --target-spv=spv1.4 -fshader-stage=frag shaders/frag.glsl -o bin/frag.spv
	glslc --target-spv=spv1.4 -fshader-stage=vert shaders/vert.glsl -o bin/vert.spv
	g++ -std=c++23 src/*.cpp -I../VkZero/include -o bin/voxels -g3 -L../VkZero -lvkzero  -Wl,-rpath,../VkZero