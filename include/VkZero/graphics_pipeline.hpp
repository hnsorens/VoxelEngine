#pragma once

#include "VkZero/Internal/shader_group_internal.hpp"
#include "VkZero/Internal/shader_resource_set_internal.hpp"
#include "shader_group.hpp"
#include "shader_resource_set.hpp"
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VkZero {
namespace PipelineDetails {

template <int Stage, typename ShaderBinding, typename Resource>
constexpr bool resource_matches() {
  return ShaderBinding::get_binding() == Resource::get_binding() &&
         ShaderBinding::get_descriptor_count() ==
             Resource::get_descriptor_count() &&
         ShaderBinding::type() == Resource::type() &&
         (Stage & Resource::get_stages()) != 0;
}

template <int Stage, typename ShaderBinding, typename ResourceTuple>
constexpr bool is_binding_valid() {
  return
      []<typename... Resources>(std::type_identity<std::tuple<Resources...>>) {
        return (resource_matches<Stage, ShaderBinding, Resources>() || ...);
      }(std::type_identity<ResourceTuple>{});
}

template <int Stage, int SetIndex, typename ShaderBinding, typename SetTuple>
constexpr bool cross_set_binding_validity_checker() {
  return []<typename... Sets>(std::type_identity<std::tuple<Sets...>>) {
    return (is_binding_valid<Stage, ShaderBinding,
                             typename Sets::BindingResources>() &&
            ...);
  }(std::type_identity<SetTuple>{});
}

template <int Stage, typename Sets, typename ShaderBindingsTuple>
constexpr bool shader_binding_validity_checker() {
  return []<typename... ShaderBindings>(
             std::type_identity<std::tuple<ShaderBindings...>>) {
    return !(
        cross_set_binding_validity_checker<Stage, 0, ShaderBindings, Sets>() &&
        ...);
  }(std::type_identity<ShaderBindingsTuple>{});
}

template <typename Sets, typename ShaderTuple>
constexpr bool shader_group_validity_checker() {
  return []<typename... Shaders>(std::type_identity<std::tuple<Shaders...>>) {
    return !(
        shader_binding_validity_checker<Shaders::get_type(), Sets,
                                        typename Shaders::BindingsList>() &&
        ...);
  }(std::type_identity<ShaderTuple>{});
}
template <typename ShaderGroup, typename... ResourceSets>
concept graphics_pipeline_validator =
    shader_group_validity_checker<std::tuple<ResourceSets...>,
                                  typename ShaderGroup::shaders>();
} // namespace PipelineDetails


struct GraphicsPipelineBase {
  GraphicsPipelineBase(ShaderGroupImpl *shaderGroup,
                       std::vector<ShaderResourceSetImpl_T *> resources);

  struct GraphicsPipelineImpl_T *impl;
};

template <typename ShaderGroup, typename... ShaderResourcesBindings>
class GraphicsPipeline : public GraphicsPipelineBase {
  // Validate that the pipeline configuration is valid at compile time
  static_assert(PipelineDetails::graphics_pipeline_validator<
                    ShaderGroup, ShaderResourcesBindings...>,
                "Graphics Pipeline Invalid");

public:
  using Attachments = ShaderGroup::Attachments;

  /**
   * @brief Constructs a graphics pipeline
   * @param ctx Vulkan context for device access
   * @param shaderGroup The shader group containing all shaders for this
   * pipeline
   * @param resources... The resource sets to bind to this pipeline
   *
   * This constructor creates the pipeline layout and prepares the pipeline
   * for creation. The actual pipeline is created when create_pipeline() is
   * called.
   */
  GraphicsPipeline(ShaderGroup &shaderGroup,
                   ShaderResourcesBindings &...resources)
      : GraphicsPipelineBase(shaderGroup.impl, {resources.impl...}) {}
};
} // namespace VkZero
