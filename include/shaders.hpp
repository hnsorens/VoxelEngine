#pragma once


#include "RenderPass.hpp"
#include "ShaderBuilder.hpp"
#include <vulkan/vulkan_core.h>
#include "Binding.hpp"

  struct RaytracingPushConstant {
    uint32_t flag;
    uint32_t frame;
  };

using main_vert = Shader<
  "main_vert",
  "bin/vert.spv",
  SHADER_VERTEX
>;

using main_frag = Shader<
  "main_frag",
  "bin/frag.spv",
  SHADER_FRAGMENT,
  ShaderBindings<
    ShaderBinding<class ShaderImage, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 0, 0, 1>
  >,
  ShaderAttachments<
    ColorAttachment<"output", VK_FORMAT_B8G8R8A8_SRGB, 0>
  >
>;

using main_rmiss = Shader<
  "main_rmiss",
  "bin/rmiss.spv",
  SHADER_RMISS
>;

using main_rgen = Shader<
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
  >,
  ShaderPushConstant<RaytracingPushConstant>
>;

using GlobalShaderTypes = ShaderTypes<
  main_vert,
  main_frag,
  main_rmiss,
  main_rgen
>;