#pragma once

#include "VkZero/types.hpp"
#include <cstring>
#include <type_traits>
#include <vector>

namespace VkZero {

template <typename Resource, DescriptorType ResourceType, int BindingSet,
          int Binding, int DescriptorCount>
struct ShaderBinding {
  using infoType = Resource *;
  static constexpr DescriptorType type() { return ResourceType; }

  static constexpr int get_binding_set() { return BindingSet; }
  static constexpr int get_binding() { return Binding; }
  static constexpr int get_descriptor_count() { return DescriptorCount; }
};


struct ResourceBindingBase {
public:
  ResourceBindingBase(std::vector<struct BindResource *> resources,
                      DescriptorType type, uint32_t descriptorCount,
                      uint32_t binding, uint32_t stages);

  struct ResourceBindingImpl_T* impl;

};

template <typename Resource, DescriptorType ResourceType, int Stages,
          int Binding, int DescriptorCount>
struct ResourceBinding : public ResourceBindingBase {
  using resourceType = Resource *;
  static constexpr int get_stages() { return Stages; }
  static constexpr DescriptorType type() { return ResourceType; }
  static constexpr int get_binding() { return Binding; }
  static constexpr int get_descriptor_count() { return DescriptorCount; }

  template <std::size_t... Is>
  static constexpr auto create_init_list(Resource *info,
                                         std::index_sequence<Is...>) {
    return std::initializer_list<struct BindResource *>{
        static_cast<struct BindResource *>(&info[Is])...};
  }

  ResourceBinding(Resource *info)
      : ResourceBindingBase(
            [&]() {
              std::vector<struct BindResource*> bindings;
              for (int i = 0; i < DescriptorCount; ++i) {
                bindings.push_back((BindResource*)info[i].impl);
              }
              return std::move(bindings);
            }(),
            ResourceType, DescriptorCount, Binding, Stages) {}
};

} // namespace VkZero
