#pragma once


#include "ShaderBuilder.hpp"

using GlobalShaderTypes = ShaderTypes<
  Shader<
    "main_vert",
    "bin/vert.spv",
    SHADER_VERTEX
  >,
  Shader<
    "main_frag",
    "bin/frag.spv",
    SHADER_FRAGMENT,
    ShaderBinding<class Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1, 1>
  >
>;