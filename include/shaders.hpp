#pragma once


#include "ShaderBuilder.hpp"

using GlobalShaderTypes = ShaderTypes<
  Shader<
    "main_vert",
    "bin/vert.spv",
    SHADER_VERTEX,
    Binding<class Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, 1>
  >,
  Shader<
    "main_frag",
    "bin/frag.spv",
    SHADER_FRAGMENT,
    Binding<class Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1, 1>
  >
>;