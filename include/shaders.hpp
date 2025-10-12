#pragma once


#include "VkZero/render_pass.hpp"
#include <vulkan/vulkan_core.h>
#include "VkZero/binding.hpp"
#include "VkZero/shader_library.hpp"
#include "VkZero/types.hpp"

struct RaytracingPushConstant {
  uint32_t flag;
  uint32_t frame;
};

using main_vert = VkZero::Shader<
  "main_vert",
  "bin/vert.spv",
  VkZero::SHADER_VERTEX
>;

using main_frag = VkZero::Shader<
  "main_frag",
  "bin/frag.spv",
  VkZero::SHADER_FRAGMENT,
  VkZero::ShaderBindings<
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::CombinedImageSampler, 0, 0, 1>
  >,
  VkZero::ShaderAttachments<
    VkZero::ColorAttachment<"output", VK_FORMAT_B8G8R8A8_SRGB, 0>
  >
>;

using main_rmiss = VkZero::Shader<
  "main_rmiss",
  "bin/rmiss.spv",
  VkZero::SHADER_RMISS
>;

using main_rgen = VkZero::Shader<
  "main_rgen",
  "bin/rgen.spv",
  VkZero::SHADER_RGEN,
  VkZero::ShaderBindings<
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 0, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::UniformBuffer, 0, 1, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 2, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 3, 512>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 4, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 5, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 6, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 7, 1>,
    VkZero::ShaderBinding<class ShaderImage, VkZero::DescriptorType::StorageImage, 0, 8, 1>
  >,
  VkZero::ShaderPushConstant<RaytracingPushConstant>
>;

using GlobalShaderLibrary = VkZero::ShaderLibrary<
  main_vert,
  main_frag,
  main_rmiss,
  main_rgen
>;