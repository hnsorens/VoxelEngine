#pragma once

#include "VkZero/shader.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/Internal/resource_manager_internal.hpp"
#include <string>
#include <vulkan/vulkan_core.h>
namespace VkZero
{
    struct ShaderImpl_T
    {
        ShaderImpl_T(std::string path, VkShaderStageFlagBits type);
        VkShaderModule createShaderModule(VkDevice device, const std::vector<char> &code);


        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo shaderInfo {};
    };
}