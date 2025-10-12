#pragma once

#include "VkZero/info.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

namespace VkZero
{

struct UniformBufferBase
{
    UniformBufferBase(void* ubo, size_t size);
    void update(int currentFrame);

    struct UniformBufferImpl_T* impl;
};

template <typename Structure>
class UniformBuffer : public UniformBufferBase {
public:
  UniformBuffer(Structure &ubo) : UniformBufferBase(&ubo, sizeof(Structure)) {}
};

}