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
  >,
  Shader<
    "main_rmiss",
    "bin/rmiss.spv",
    SHADER_RMISS
  >,
  Shader<
    "main_rgen",
    "bin/rgen.spv",
    SHADER_RGEN,
    ShaderBindings<
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 0, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, 1, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 2, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 3, 512>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 4, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 5, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 6, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 7, 1>,
      ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 0, 8, 1>
    >
  >
>;