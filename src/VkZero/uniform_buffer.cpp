#include "VkZero/uniform_buffer.hpp"
#include "VkZero/Internal/uniform_buffer_internal.hpp"

using namespace VkZero;

UniformBufferBase::UniformBufferBase(void* ubo, size_t size)
{
    impl = new UniformBufferImpl_T(ubo, size);
}

void UniformBufferBase::update(int currentFrame)
{
  impl->update(currentFrame);
}

UniformBufferImpl_T::UniformBufferImpl_T(void* ubo, size_t size) : size(size), ubo(ubo)
{
    uniformBuffer.resize(MAX_FRAMES_IN_FLIGHT);
uniformBufferMemory.resize(MAX_FRAMES_IN_FLIGHT);
uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
  VkDeviceSize bufferSize = size;

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
              size, 0, &uniformBuffersMapped[i]);
  std::memcpy(uniformBuffersMapped[i], &ubo, size);
}
}

void UniformBufferImpl_T::writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet,
                  uint32_t binding, uint32_t element,
                  VkDescriptorType type, int frame) {

}

void UniformBufferImpl_T::update(int currentFrame)
{
  std::memcpy(uniformBuffersMapped[currentFrame], ubo, size);
}