#pragma once

namespace VkZero
{
    enum ShaderType
    {
        SHADER_VERTEX = 0x00000001,
        SHADER_FRAGMENT = 0x00000010,
        SHADER_GEOMETRY = 0x00000008,
        SHADER_RMISS = 0x00000800,
        SHADER_RGEN = 0x00000100,
    };
}
