#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "VkZero/resource_manager.hpp"
#include <stdexcept>
#include <vector>

struct UniformBufferImpl_T
{
    UniformBufferImpl_T(void* ubo)
    {

    }
    
    void* ubo;
};

struct UniformBufferBase
{
    UniformBufferBase(void* ubo)
    {
        impl = new UniformBufferImpl_T(ubo);
    }

    struct UniformBufferImpl_T* impl;
};

template <typename Structure>
class UniformBuffer : public VkZero::BindResource {
  UniformBuffer(Structure &ubo) : ubo(ubo) {}

  void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                       uint32_t binding, uint32_t element,
                       VkDescriptorType type, int frame) override {

    uniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
      VkDeviceSize bufferSize = sizeof(Structure);

      VkBufferCreateInfo bufferInfo = {};
      bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      bufferInfo.size = bufferSize;
      bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

      if (vkCreateBuffer(VkZero::vkZero_core->device, &bufferInfo, nullptr,
                         &uniformBuffer[i]) != VK_SUCCESS) {
        throw std::runtime_error("failed to create uniform buffer");
      }

      VkMemoryRequirements memRequirements;
      vkGetBufferMemoryRequirements(VkZero::vkZero_core->device,
                                    uniformBuffer[i], &memRequirements);

      VkMemoryAllocateInfo allocInfo{};
      allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      allocInfo.allocationSize = memRequirements.size;
      allocInfo.memoryTypeIndex = VkZero::ResourceManager::findMemoryType(
          VkZero::vkZero_core->physicalDevice, memRequirements.memoryTypeBits,
          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

      if (vkAllocateMemory(VkZero::vkZero_core->device, &allocInfo, nullptr,
                           &uniformBufferMemory[i]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate uniform buffer memory!");
      }

      vkBindBufferMemory(VkZero::vkZero_core->device, uniformBuffer[i],
                         uniformBufferMemory[i], 0);

      vkMapMemory(VkZero::vkZero_core->device, uniformBufferMemory[i], 0,
                  sizeof(Structure), 0, &uniformBuffersMapped[i]);
      memcpy(uniformBuffersMapped[i], &ubo, sizeof(Structure));
    }
  }

  Structure &ubo;

  std::vector<VkBuffer> uniformBuffer;
  std::vector<VkDeviceMemory> uniformBufferMemory;

  std::vector<void *> uniformBuffersMapped;
};