glslc --target-spv=spv1.4 -fshader-stage=rgen rgen.glsl -o rgen.spv
glslc --target-spv=spv1.4 -fshader-stage=rmiss rmiss.glsl -o rmiss.spv
glslc --target-spv=spv1.4 -fshader-stage=frag frag.glsl -o frag.spv
glslc --target-spv=spv1.4 -fshader-stage=vert vert.glsl -o vert.spv
g++ main.cpp WindowManager.cpp -o voxels -lvulkan -lglfw -lglm