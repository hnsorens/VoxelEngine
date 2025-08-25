#pragma once


#include "RenderPass.hpp"
#include "ShaderBuilder.hpp"
#include <vulkan/vulkan_core.h>
#include "Binding.hpp"

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
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0, 1>
    >,
    ShaderAttachments<
      ColorAttachment<"output", VK_FORMAT_B8G8R8A8_SRGB, 0>
    >
  >
>;