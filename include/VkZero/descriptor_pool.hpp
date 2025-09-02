#pragma once

#include <vulkan/vulkan_core.h>

namespace VkZero
{
    class DescriptorPool
    {
    public:
        DescriptorPool(const DescriptorPool&) = delete;
        DescriptorPool& operator=(const DescriptorPool&) = delete;

        static DescriptorPool& instance() {
            static DescriptorPool instance;
            return instance;
        }

        VkDescriptorPool get()
        {
            return pool;
        }

    private:
        VkDescriptorPool pool;
        DescriptorPool();
    };
}