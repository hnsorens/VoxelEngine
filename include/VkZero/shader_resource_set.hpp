#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/binding.hpp"
#include "VkZero/descriptor_pool.hpp"
#include "VkZero/info.hpp"
#include <memory>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VkZero {
namespace ShaderResourceSetDetails {
/**
 * @brief Checks if any binding types have duplicate binding numbers within a
 * resource set
 * @tparam Bindings... Variadic template of binding types to check
 * @return true if duplicates are found, false otherwise
 *
 * This ensures that each binding in a resource set has a unique binding number.
 * Duplicate binding numbers would cause Vulkan descriptor set creation to fail.
 */
template <typename... Bindings> struct duplicate_binding_number_checker;

// Base case: no bindings to check
template <> struct duplicate_binding_number_checker<> {
  static constexpr bool value = false;
};

// Recursive case: check first binding against all remaining bindings
template <typename First, typename... Rest>
struct duplicate_binding_number_checker<First, Rest...> {
  static constexpr bool value =
      ((First::get_binding() == Rest::get_binding()) || ...) ||
      duplicate_binding_number_checker<Rest...>::value;
};
} // namespace ShaderResourceSetDetails

struct ShaderResourceSetBase {
public:
  ShaderResourceSetBase(std::vector<ResourceBindingImpl_T *> bindings);

  struct ShaderResourceSetImpl_T *impl;
};

/**
 * @brief Manages a set of shader resource bindings and their corresponding
 * Vulkan descriptor set
 * @tparam Bindings... Variadic template of binding types to include in this
 * resource set
 *
 * This class creates and manages a Vulkan descriptor set layout and descriptor
 * sets for a collection of shader resource bindings. It handles the creation of
 * descriptor set layouts with proper binding flags and allocates descriptor
 * sets from the global pool.
 */
template <typename... Bindings>
class ShaderResourceSet : public ShaderResourceSetBase {
  // Ensure no duplicate binding numbers in this resource set
  static_assert(!ShaderResourceSetDetails::duplicate_binding_number_checker<
                    Bindings...>::value,
                "Shader Resource Set cannot have duplicate bindings!");

public:
  using BindingResources = std::tuple<Bindings...>;

  /**
   * @brief Constructs a ShaderResourceSet with the given bindings
   * @param bindings... The binding instances to include in this resource set
   *
   * This constructor:
   * 1. Creates a descriptor set layout with all the bindings
   * 2. Allocates descriptor sets from the global descriptor pool
   * 3. Writes all binding data to the descriptor sets
   */
  ShaderResourceSet(Bindings &&...bindings)
      : ShaderResourceSetBase({bindings.impl...}) {}
};
} // namespace VkZero
