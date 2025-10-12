# Makefile for VoxelEngine

all:
	ctags -R
	glslc --target-spv=spv1.4 -fshader-stage=rgen shaders/rgen.glsl -o bin/rgen.spv
	glslc --target-spv=spv1.4 -fshader-stage=rmiss shaders/rmiss.glsl -o bin/rmiss.spv
	glslc --target-spv=spv1.4 -fshader-stage=frag shaders/frag.glsl -o bin/frag.spv
	glslc --target-spv=spv1.4 -fshader-stage=vert shaders/vert.glsl -o bin/vert.spv
	g++ -std=c++23 src/*.cpp -Iinclude -IVkZero/include -o bin/voxels -g3 -LVkZero -lvkzero  -Wl,-rpath,./VkZero

clean:
	rm -rf bin/* shaders/*.spv

run:
	./bin/voxels 
