#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/Internal/graphics_pipeline_internal.hpp"
#include "VkZero/Internal/image_internal.hpp"
#include "VkZero/Internal/raytracing_pipeline_internal.hpp"
#include "VkZero/fixed_string.hpp"
#include "VkZero/window.hpp"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

#define ALIGN_UP(value, alignment)                                             \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

namespace VkZero {


struct RenderPassResourceBase {
 RenderPassResourceBase(const char* name, AttachmentImage *image);

  struct RenderPassResourceImpl_T *impl;
};

template <FixedString Name>
class RenderPassResource : public RenderPassResourceBase {
public:
  RenderPassResource(AttachmentImage *image) : RenderPassResourceBase(name, image) {}

  static constexpr const char* name = Name.value;
};

namespace RenderPassResourceSetDetails {
// Compile-time name matching to find resource index
template <FixedString TargetName, typename Tuple, std::size_t Index = 0>
constexpr std::size_t findResourceIndex() {
  if constexpr (Index >= std::tuple_size_v<Tuple>) {
    static_assert(Index < std::tuple_size_v<Tuple>,
                  "Resource not found for attachment");
    return Index;
  } else {
    using Element = std::tuple_element_t<Index, Tuple>;
    if constexpr (Element::name == TargetName) {
      return Index;
    } else {
      return findResourceIndex<TargetName, Tuple, Index + 1>();
    }
  }
}
}; // namespace RenderPassResourceSetDetails
//

struct RenderPassResourceSetBase {
  RenderPassResourceSetBase(std::vector<RenderPassResourceImpl_T *> resources);

  struct RenderPassResourceSetImpl_T *impl;
};

template <typename... Resources>
class RenderPassResourceSet : public RenderPassResourceSetBase {
public:
  RenderPassResourceSet(Resources... resources)
      : RenderPassResourceSetBase({resources.impl...}) {}
};

struct AttachmentBase {
  enum class AttachmentType {
    ATTACHMENT_COLOR,
    ATTACHMENT_DEPTH,
    ATTACHMENT_PRESERVE,
    ATTACHMENT_INPUT,
  };


AttachmentBase(const char *name, VkFormat format, int location,
                               AttachmentType type);

AttachmentBase(const char *name);
  struct AttachmentImpl_T *impl;
};



template <FixedString Name, VkFormat Format, int Location>
struct ColorAttachment : public AttachmentBase {

  ColorAttachment()
      : AttachmentBase(name.value, Format, Location,
                       AttachmentBase::AttachmentType::ATTACHMENT_COLOR) {}

  static constexpr FixedString name = Name.value;
  static constexpr VkFormat get_format() { return Format; }
  static constexpr int get_location() { return Location; }
};

template <FixedString Name, VkFormat Format, int Location>
struct DepthAttachment : public AttachmentBase {

  DepthAttachment()
      : AttachmentBase(name.value, Format, Location,
                       AttachmentBase::AttachmentType::ATTACHMENT_DEPTH) {}

  static constexpr FixedString name = Name.value;
  static constexpr VkFormat get_format() { return Format; }
  static constexpr int get_location() { return Location; }
};

template <FixedString Name> struct PreserveAttachment : public AttachmentBase {

  PreserveAttachment() : AttachmentBase(name.value) {}

  static constexpr FixedString name = Name.value;
};

template <FixedString Name, VkFormat Format, int Location>
struct InputAttachment : public AttachmentBase {

  InputAttachment()
      : AttachmentBase(name.value, Format, Location,
                       AttachmentBase::AttachmentType::ATTACHMENT_INPUT) {}

  static constexpr FixedString name = Name.value;
  static constexpr VkFormat get_format() { return Format; }
  static constexpr int get_location() { return Location; }
};
namespace RenderPassDetails {
template <typename Pipelines> struct get_all_attachments;

template <> struct get_all_attachments<std::tuple<>> {
  using value = std::tuple<>;
};

template <typename First, typename... Rest>
struct get_all_attachments<std::tuple<First, Rest...>> {
  using value = decltype(std::tuple_cat(
      std::declval<typename First::Attachments>(),
      std::declval<
          typename get_all_attachments<std::tuple<Rest...>>::value>()));
};

// Helper to append to a tuple
template <typename Tuple, typename T> struct tuple_push_back;

template <typename... Ts, typename T>
struct tuple_push_back<std::tuple<Ts...>, T> {
  using type = std::tuple<Ts..., T>;
};

// Check if a tuple contains an attachment with same name/type (for
// InputAttachment, also check format)
template <typename T, typename Tuple> struct contains_attachment;

template <typename T>
struct contains_attachment<T, std::tuple<>> : std::false_type {};

template <typename T, typename First, typename... Rest>
struct contains_attachment<T, std::tuple<First, Rest...>>
    : std::conditional_t<
          std::is_same_v<T, First> ||
              (T::name == First::name && std::is_same_v<T, First>) ||
              (std::is_same_v<T, InputAttachment<T::name, T::get_format(),
                                                 T::get_location()>> &&
               std::is_same_v<First,
                              InputAttachment<First::name, First::get_format(),
                                              First::get_location()>> &&
               T::get_format() == First::get_format()),
          std::true_type, contains_attachment<T, std::tuple<Rest...>>> {};

// Meta-function to filter preserve/input attachments
template <typename Attachments, typename Tuple> struct filter_preserve_input;

template <typename Tuple> struct filter_preserve_input<std::tuple<>, Tuple> {
  using type = std::tuple<>;
};

template <typename First, typename... Rest, typename All>
struct filter_preserve_input<std::tuple<First, Rest...>, All> {
  using tail = typename filter_preserve_input<std::tuple<Rest...>, All>::type;
  using type = std::conditional_t<
      std::is_same_v<First, PreserveAttachment<First::name>> ||
          std::is_same_v<First,
                         InputAttachment<First::name, First::get_format(),
                                         First::get_location()>>,
      std::conditional_t<contains_attachment<First, All>::value,
                         typename tuple_push_back<tail, First>::type, tail>,
      typename tuple_push_back<tail, First>::type>;
};

// Assign global locations
template <typename Tuple, int Start = 0> struct assign_global_locations;

template <int Start> struct assign_global_locations<std::tuple<>, Start> {
  using type = std::tuple<>;
};

template <typename First, typename... Rest, int Start>
struct assign_global_locations<std::tuple<First, Rest...>, Start> {
  // Produce new type with updated location
  using new_first = First;
  template <typename T> struct update_location {
    using type = T;
  };

  template <FixedString N, VkFormat F, int L>
  struct update_location<ColorAttachment<N, F, L>> {
    using type = ColorAttachment<N, F, Start>;
  };

  template <FixedString N, VkFormat F, int L>
  struct update_location<DepthAttachment<N, F, L>> {
    using type = DepthAttachment<N, F, Start>;
  };

  template <FixedString N, VkFormat F, int L>
  struct update_location<InputAttachment<N, F, L>> {
    using type = InputAttachment<N, F, Start>;
  };

  using type = decltype(std::tuple_cat(
      std::tuple<typename update_location<First>::type>{},
      typename assign_global_locations<std::tuple<Rest...>,
                                       Start + 1>::type{}));
};

// The main meta-function
template <typename AllAttachments> struct get_common_attachments {
  // Filter preserves/inputs
  using filtered =
      typename filter_preserve_input<AllAttachments, AllAttachments>::type;

  // Assign global locations
  using value = typename assign_global_locations<filtered>::type;
};
}; // namespace RenderPassDetails

// Compile-time tuple iteration
template <typename Tuple, typename Func, std::size_t... I>
constexpr void tuple_for_each_impl(Tuple &&t, Func &&f,
                                   std::index_sequence<I...>) {
  (f(std::get<I>(t)), ...);
}

template <typename Tuple, typename Func>
constexpr void tuple_for_each(Tuple &&t, Func &&f) {
  tuple_for_each_impl(
      std::forward<Tuple>(t), std::forward<Func>(f),
      std::make_index_sequence<
          std::tuple_size<std::remove_reference_t<Tuple>>::value>{});
}


struct RenderPassBase {
  RenderPassBase(uint32_t width, uint32_t height,
                 RenderPassResourceSetImpl_T *resources,
                 std::vector<GraphicsPipelineImpl_T *> pipelines,
                 std::vector<AttachmentImpl_T*> requiredAttachments);

  struct RenderPassImpl_T *impl;
};

template <typename Resources, typename... Pipelines>
class RenderPass : public RenderPassBase {
public:
  using allAttachments =
      RenderPassDetails::get_all_attachments<std::tuple<Pipelines...>>::value;
  using commonAttachments =
      RenderPassDetails::get_common_attachments<allAttachments>::value;

  static_assert(std::tuple_size<allAttachments>::value > 0,
                "RenderPass must have at least one attachment.");

  RenderPass(uint32_t width, uint32_t height, Resources &resource,
             Pipelines &...pipelines)
      : RenderPassBase(width, height, resource.impl, {pipelines.impl...},
                       GetAttachments<commonAttachments>::get()) {}
};

template <typename... Structures> struct PushConstantData {
  PushConstantData() {
    size_t size = (sizeof(Structures) + ...);
    data = new char(size);
  }

  ~PushConstantData() { delete data; }

  template <typename T> T *get() { return (T *)(data + offset<T>()); }

private:
  template <typename T> constexpr size_t offset() {
    size_t current = 0;
    (
        [&]() {
          if constexpr (std::is_same<T, Structures>()) {
            return current;
          } else {
            current += sizeof(Structures);
          }
        }(),
        ...);

    static_assert("Type does not exist in Push Constant Data");
    return 0;
  }

  char *data;

  template <typename... RaytracingPipelines> friend class RaytracingRenderPass;
};

template <typename PushConstant, typename Pipeline>
struct RaytracingRenderPassPipeline {
public:
  RaytracingRenderPassPipeline(Pipeline &pipeline, PushConstant &pushConstants)
      : pipeline(pipeline), pushConstantData(pushConstants) {}

private:
  Pipeline &pipeline;
  PushConstant &pushConstantData;

  template <typename... RaytracingPipelines> friend class RaytracingRenderPass;
};

template <typename... RaytracingPipelines> class RaytracingRenderPass {
public:
  RaytracingRenderPass(RaytracingPipelines... pipelines)
      : pipelines{pipelines...} {
    vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(
        vkGetDeviceProcAddr(vkZero_core->device, "vkCmdTraceRaysKHR"));
  }

  void record(VkCommandBuffer commandBuffer, uint32_t currentFrame,
              uint32_t imageIndex) {
    std::apply(
        [&](auto &pipeline) {
          vkCmdBindPipeline(commandBuffer,
                            VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                            pipeline.pipeline.impl->pipeline);
          pipeline.pipeline.impl->bindResources(commandBuffer, currentFrame);
          *(uint32_t *)(pipeline.pushConstantData.data) = 0;
          vkCmdPushConstants(commandBuffer,
                             pipeline.pipeline.impl->pipelineLayout,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 8,
                             pipeline.pushConstantData.data);
          vkCmdTraceRaysKHR(commandBuffer,
                            &pipeline.pipeline.impl->raygenRegion,
                            &pipeline.pipeline.impl->missRegion,
                            &pipeline.pipeline.impl->hitRegion,
                            &pipeline.pipeline.impl->callableRegion,
                            RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);

          VkMemoryBarrier barrier = {};
          barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
          barrier.srcAccessMask =
              VK_ACCESS_SHADER_WRITE_BIT |
              VK_ACCESS_SHADER_READ_BIT; // Ensure writes from the first trace
                                         // finish
          barrier.dstAccessMask =
              VK_ACCESS_SHADER_READ_BIT |
              VK_ACCESS_SHADER_WRITE_BIT; // Ensure the second trace can read
                                          // them
          barrier.pNext = 0;
          vkCmdPipelineBarrier(
              commandBuffer,
              VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First
                                                            // trace rays
                                                            // execution
              VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination:
                                                            // Second trace rays
                                                            // execution
              0, 1, &barrier, 0, nullptr, 0, nullptr);
          *(uint32_t *)(pipeline.pushConstantData.data) = 1;
          vkCmdPushConstants(commandBuffer,
                             pipeline.pipeline.impl->pipelineLayout,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4,
                             pipeline.pushConstantData.data);
          vkCmdTraceRaysKHR(commandBuffer,
                            &pipeline.pipeline.impl->raygenRegion,
                            &pipeline.pipeline.impl->missRegion,
                            &pipeline.pipeline.impl->hitRegion,
                            &pipeline.pipeline.impl->callableRegion,
                            RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);
          vkCmdPipelineBarrier(
              commandBuffer,
              VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First
                                                            // trace rays
                                                            // execution
              VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination:
                                                            // Second trace rays
                                                            // execution
              0, 1, &barrier, 0, nullptr, 0, nullptr);
          *(uint32_t *)(pipeline.pushConstantData.data) = 2;
          vkCmdPushConstants(commandBuffer,
                             pipeline.pipeline.impl->pipelineLayout,
                             VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4,
                             pipeline.pushConstantData.data);
          vkCmdTraceRaysKHR(commandBuffer,
                            &pipeline.pipeline.impl->raygenRegion,
                            &pipeline.pipeline.impl->missRegion,
                            &pipeline.pipeline.impl->hitRegion,
                            &pipeline.pipeline.impl->callableRegion,
                            RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);
        },
        pipelines);
  }

  PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
  std::tuple<RaytracingPipelines...> pipelines;
};
} // namespace VkZero
