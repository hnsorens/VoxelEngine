#include "VkZero/Internal/descriptor_pool_internal.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/types.hpp"
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vulkan/vulkan_core.h>

using namespace VkZero;

#define MAX_POOL_SETS 1000
#define MAX_DESCRIPTOR_COUNT 2000

using namespace VkZero;

DescriptorPool::DescriptorPool(){}

void DescriptorPool::build(std::vector<DescriptorType> types)
{
    std::vector<VkDescriptorPoolSize> poolSizes;
    for (auto t : types)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = (VkDescriptorType)t;
        poolSize.descriptorCount = MAX_DESCRIPTOR_COUNT;
        poolSizes.push_back(poolSize);
    }

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.maxSets = MAX_POOL_SETS;
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    if (vkCreateDescriptorPool(vkZero_core->device, &createInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create raytracing descriptor pool!");
    }
}