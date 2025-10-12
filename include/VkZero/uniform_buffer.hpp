#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "VkZero/resource_manager.hpp"
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