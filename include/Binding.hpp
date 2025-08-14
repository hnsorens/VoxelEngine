#pragma once

#include "BindResource.hpp"
#include <cstring>
#include <type_traits>
#include <vector>


template <typename Resource, VkDescriptorType ResourceType, int BindingCount, int Count>
struct Binding
{
    // static_assert(std::is_base_of_v<BindResource, Resource>,
    //               "Resource must be derived from BindResource");

    using infoType = Resource*;
    static constexpr VkDescriptorType type() { return ResourceType; }
    static constexpr int get_binding() { return BindingCount; }
    static constexpr int get_descriptor_count() { return Count; }

    VkDescriptorType bindingType() { return type(); }
    int binding() { return get_binding(); }
    // using type = Info;

    Binding(VkDevice device, std::vector<VkDescriptorSet>& descriptors, Resource* info[BindingCount])
    {
        std::memcpy(resources, info, BindingCount * sizeof(Resource*));
    }

    void write(VkDevice device, VkDescriptorSet& descriptorSet, int element, int frame)
    {
        VkWriteDescriptorSet descriptorWrite;
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSet;
        descriptorWrite.dstBinding = binding();
        descriptorWrite.dstArrayElement = element;
        descriptorWrite.descriptorType = type();
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pNext = nullptr;
        resources[element]->write(descriptorWrite, element);

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    Resource* resources[BindingCount];
};

template <typename T>
inline constexpr bool is_binding_v =
    std::is_base_of_v<Binding<typename T::infoType, T::type(), T::get_binding(), T::get_descriptor_count()>, T>;

template <int Binding, int DescriptorCount>
struct BindingSlot {
    template <typename T>
    struct Bind {
        using infoType = T;
        T info[DescriptorCount];  // Value storage
        
        // Copy array constructor
        Bind(const T (&arr)[DescriptorCount]) {
            for (int i = 0; i < DescriptorCount; ++i) {
                info[i] = arr[i];  // Requires T to be copyable
            }
        }
        
        // Single-element copy constructor
        Bind(const T& value) requires (DescriptorCount == 1)
            : info{value} {}  // Direct initialization
        
        static constexpr int get_binding() { return Binding; }
        static constexpr int get_descriptor() { return DescriptorCount; }
    };
};

template <typename type, int binding>
struct Bind {
    static_assert(is_binding_v<type>);
    using infoType = typename type::infoType;
    static constexpr VkDescriptorType get_type() { return type::type(); }
    static constexpr int get_binding() { return binding; }
};

template <int binding, typename Tuple>
struct find_binding;

template <int binding, typename First, typename... Rest>
struct find_binding<binding, std::tuple<First, Rest...>> {
    using type = typename std::conditional_t<
        (First::get_binding() == binding),
        First,
        typename find_binding<binding, std::tuple<Rest...>>::type
    >;
};

template <int binding>
struct find_binding<binding, std::tuple<>> {
    using type = void;
};

// Check if a binding exists in a tuple
template <int binding, typename Tuple>
struct has_binding {
    static constexpr bool value = 
        !std::is_same_v<typename find_binding<binding, Tuple>::type, void>;
};

// Get the type of a binding in a tuple
template <int binding, typename Tuple>
using get_binding_type = typename find_binding<binding, Tuple>::type;

// Combine bindings from multiple shaders
template <typename... Shaders>
struct CombinedBindings {
    using AllBindings = decltype(std::tuple_cat(
        std::declval<typename Shaders::BindingsList>()...
    ));
    
    template <typename Tuple, typename Result = std::tuple<>>
    struct filter_duplicates;
    
    template <typename... Result>
    struct filter_duplicates<std::tuple<>, std::tuple<Result...>> {
        using type = std::tuple<Result...>;
    };
    
    template <typename First, typename... Rest, typename... Result>
    struct filter_duplicates<std::tuple<First, Rest...>, std::tuple<Result...>> {
        static constexpr int current_binding = First::get_binding();
        using type = typename std::conditional_t<
            has_binding<current_binding, std::tuple<Result...>>::value,
            typename filter_duplicates<std::tuple<Rest...>, std::tuple<Result...>>::type,
            typename filter_duplicates<std::tuple<Rest...>, std::tuple<Result..., First>>::type
        >;
    };
    
    using type = typename filter_duplicates<AllBindings>::type;
};