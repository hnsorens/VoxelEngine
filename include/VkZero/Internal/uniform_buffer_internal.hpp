#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/uniform_buffer.hpp"
#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "VkZero/resource_manager.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace VkZero
{


struct UniformBufferImpl_T : public BindResource
{
  UniformBufferImpl_T(void* ubo, size_t size);

  void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                    uint32_t binding, uint32_t element,
                    VkDescriptorType type, int frame) override;

  void update(int currentFrame);
  
  size_t size;
  void* ubo;

  std::vector<VkBuffer> uniformBuffer;
  std::vector<VkDeviceMemory> uniformBufferMemory;
  std::vector<void *> uniformBuffersMapped;
};

}