#pragma once

#include "VkZero/Internal/shader_internal.hpp"
#include "shader.hpp"
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>
namespace VkZero {
namespace ShaderGroupDetails {
/**
 * @brief Validates that all shader bindings in a shader group are compatible
 * @tparam Shaders... Variadic template of shader types to validate
 * @return true if all bindings are compatible, false if conflicts exist
 *
 * This struct ensures that shaders in a group don't have conflicting bindings.
 * Two bindings conflict if they use the same binding slot but have different
 * types or descriptor counts.
 */
template <typename... Shaders> struct shader_binding_validator {
private:
  // Combine all shaders' BindingsList tuple types into one flattened tuple
  using all_bindings_tuple = decltype(std::tuple_cat(
      std::declval<typename Shaders::BindingsList>()...));

  /**
   * @brief Checks if two binding types are compatible
   * @tparam A First binding type
   * @tparam B Second binding type
   * @return true if bindings are compatible (different slots or same slot with
   * same type/count)
   */
  template <typename A, typename B> struct binding_compatibility_checker {
    static constexpr bool same_slot =
        (A::get_binding_set() == B::get_binding_set()) &&
        (A::get_binding() == B::get_binding());

    static constexpr bool same_type = A::type() == B::type();

    static constexpr bool same_count =
        (A::get_descriptor_count() == B::get_descriptor_count());

    // Bindings are compatible if they use different slots, or same slot with
    // same type and count
    static constexpr bool value = !same_slot || (same_type && same_count);
  };

  /**
   * @brief Recursively checks all bindings in a tuple for compatibility
   * @tparam Ts... Variadic template of binding types to check
   * @return true if all bindings are compatible with each other
   */
  template <typename... Ts> struct pairwise_compatibility_checker {
    static constexpr bool value = true; // Empty pack or single element is fine
  };

  template <typename First, typename... Rest>
  struct pairwise_compatibility_checker<First, Rest...> {
    static constexpr bool value =
        ((binding_compatibility_checker<First, Rest>::value) &&
         ...) && // Compare First with all Rest
        pairwise_compatibility_checker<Rest...>::value; // Then recurse
  };

  /**
   * @brief Converts a tuple of bindings into a compatibility check
   * @tparam Tuple The tuple of binding types
   * @return true if all bindings in the tuple are compatible
   */
  template <typename Tuple> struct tuple_compatibility_checker;

  template <typename... Ts>
  struct tuple_compatibility_checker<std::tuple<Ts...>> {
    static constexpr bool value = pairwise_compatibility_checker<Ts...>::value;
  };

public:
  static constexpr bool value =
      tuple_compatibility_checker<all_bindings_tuple>::value;
};

/**
 * @brief Collects and flattens all attachment types from a tuple of shaders
 * @tparam ShaderTuple The tuple of shader types to collect attachments from
 * @return A flattened tuple containing all attachment types from all shaders
 */
template <typename ShaderTuple> struct attachment_collector;

// Base case: empty tuple
template <> struct attachment_collector<std::tuple<>> {
  using value = std::tuple<>;
};

// Recursive case: concatenate attachments from first shader with rest
template <typename First, typename... Rest>
struct attachment_collector<std::tuple<First, Rest...>> {
  using value = decltype(std::tuple_cat(
      std::declval<typename First::Attachments>(),
      std::declval<
          typename attachment_collector<std::tuple<Rest...>>::value>()));
};

/**
 * @brief Validates that push constant types are compatible across shaders
 * @tparam Shader The shader type to validate against
 * @tparam ShaderTuple The tuple of other shader types to compare with
 * @return true if all push constant types are compatible
 *
 * Push constants are compatible if they are the same type, or if one is void
 */
template <typename Shader, typename ShaderTuple>
struct push_constant_compatibility_validator;

// Base case: no shaders to compare
template <typename Shader>
struct push_constant_compatibility_validator<Shader, std::tuple<>> {
  static constexpr bool value = true;
};

// Recursive case: check compatibility with first shader and continue
template <typename Shader, typename First, typename... Rest>
struct push_constant_compatibility_validator<Shader,
                                             std::tuple<First, Rest...>> {
  static constexpr bool value =
      (std::is_same<typename Shader::PushConstantType,
                    typename First::PushConstantType>() ||
       std::is_same<typename Shader::PushConstantType, void>() ||
       std::is_same<typename First::PushConstantType, void>()) &&
      push_constant_compatibility_validator<Shader, std::tuple<Rest...>>::value;
};
} // namespace ShaderGroupDetails

/**
 * @brief Represents a push constant structure for a shader
 * @tparam T The C++ structure type that represents the push constant data
 * @tparam ShaderStages The shader stages this push constant is used in
 * (bitfield)
 */
template <typename T, int ShaderStages> struct PushConstant {
  using Structure = T;
  static constexpr int shaderStages = ShaderStages;
};

struct ShaderPushConstantData {
  ShaderPushConstantData(uint32_t size, uint32_t stages) : structureSize(size), shaderStages(stages) {}
  uint32_t structureSize;
  uint32_t shaderStages;
};

struct ShaderPushConstantsBase {
  ShaderPushConstantsBase(std::vector<ShaderPushConstantData> pushConstants);

  struct ShaderPushConstantsImpl_T *impl;
};

template <typename Attachments>
struct GetAttachmentNames;

template <typename... Attachments>
struct GetAttachmentNames<std::tuple<Attachments...>>{
  static std::vector<struct AttachmentImpl_T*> get()
  {
    return std::vector<struct AttachmentImpl_T*>({Attachments{}.impl...});
  }
};

/**
 * @brief Manages multiple push constant ranges for a shader group
 * @tparam PushConstants... Variadic template of PushConstant types
 *
 * This class automatically calculates offsets and creates Vulkan push constant
 * ranges for all the push constants in a shader group.
 */
template <typename... PushConstants>
class ShaderPushConstants : public ShaderPushConstantsBase {
public:
  ShaderPushConstants()
      : ShaderPushConstantsBase({
            {sizeof(typename PushConstants::Structure), PushConstants::shaderStages}...}) {}

private:
  template <typename ShaderGroup, typename... ResourceSets>
  friend class RaytracingPipeline;
  template <typename ShaderGroup, typename... ResourceSets>
  friend class GraphicsPipeline;
  template <typename ShaderPushConstants, typename... Shaders>
  friend class ShaderGroup;
};

struct ShaderGroupBase
{
  ShaderGroupBase(std::vector<struct ShaderImpl_T*> shaders, struct ShaderPushConstantsImpl_T* pushConstants, std::vector<AttachmentImpl_T*> attachments);
  struct ShaderGroupImpl* impl;
};

  template <typename ShaderPushConstants, typename... Shaders>
  class ShaderGroup : public ShaderGroupBase {
    // Validate that all shader bindings are compatible
    static_assert(
        ShaderGroupDetails::shader_binding_validator<Shaders...>::value,
        "Shader bindings conflict - same binding number with different "
        "resource type or different binding counts!");

  public:
    using shaders = std::tuple<Shaders...>;
    using Attachments =
        ShaderGroupDetails::attachment_collector<shaders>::value;

    // Constructor: takes push constants and shader instances
    ShaderGroup(ShaderPushConstants &pushConstants, Shaders &...shaders)
        : ShaderGroupBase({shaders.impl...},
                          pushConstants.impl, GetAttachmentNames<Attachments>::get()) {}
  };
} // namespace VkZero
