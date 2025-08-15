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
      ShaderBinding<class Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1, 1>
    >,
    ShaderAttachments<
      ColorAttachment<"output", VK_FORMAT_R32_UINT, 0>
    >
  >,
   Shader<
    "main_vert2",
    "bin/vert.spv",
    SHADER_VERTEX
  >,
  Shader<
    "main_frag2",
    "bin/frag.spv",
    SHADER_FRAGMENT,
    ShaderBindings<
      ShaderBinding<class Image, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 1, 1>
    >,
    ShaderAttachments<
      ColorAttachment<"output", VK_FORMAT_R32_UINT, 0>,
      InputAttachment<"output", VK_FORMAT_R32_UINT, 1>
    >
  >
>;