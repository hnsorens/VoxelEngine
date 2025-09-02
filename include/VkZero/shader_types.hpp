#pragma once

#include <vulkan/vulkan_core.h>
#include <tuple>
#include <memory>

namespace VkZero
{
    enum ShaderType
    {
        SHADER_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
        SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
        SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
        SHADER_RMISS = VK_SHADER_STAGE_MISS_BIT_KHR,
        SHADER_RGEN = VK_SHADER_STAGE_RAYGEN_BIT_KHR,
    };

    // Forward declarations for ShaderTypes class
    template <typename... Shaders>
    class ShaderTypes;
}
