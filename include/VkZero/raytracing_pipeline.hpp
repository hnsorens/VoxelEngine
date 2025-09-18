#pragma once

#include "VkZero/Internal/shader_resource_set_internal.hpp"
#include "VkZero/resource_manager.hpp"
#include "graphics_pipeline.hpp"
#include "shader_group.hpp"
#include "shader_resource_set.hpp"
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>

#define ALIGN_UP(value, alignment)                                             \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

namespace VkZero {


struct RaytracingPipelineBase {
  RaytracingPipelineBase(ShaderGroupImpl *shaderGroup,
                         std::vector<ShaderResourceSetImpl_T *> resources);

  struct RaytracingPipelineImpl_T *impl;
};

template <typename ShaderGroup, typename... ShaderResourcesBindings>
class RaytracingPipeline : public RaytracingPipelineBase {
  static_assert(PipelineDetails::graphics_pipeline_validator<
                    ShaderGroup, ShaderResourcesBindings...>,
                "Graphics Pipeline Invalid");

public:
  using Attachments = ShaderGroup::Attachments;

  RaytracingPipeline(ShaderGroup &shaderGroup,
                     ShaderResourcesBindings &...resources)
      : RaytracingPipelineBase(shaderGroup.impl, {resources.impl...}) {}
};
} // namespace VkZero
