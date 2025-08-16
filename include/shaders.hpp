#pragma once


#include "RenderPass.hpp"
// #include "ShaderBuilder.hpp"
#include <vulkan/vulkan_core.h>

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
    ShaderBindings<
      ShaderBinding<class Image, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 1, 1>
    >,
    ShaderAttachments<
      ColorAttachment<"output", VK_FORMAT_R16G16B16A16_SFLOAT, 0>
    >
  >
>;