#pragma once

#include "VkZero/bind_resource.hpp"
#include <cstring>
#include <type_traits>
#include <vector>

namespace VkZero
{
    template <typename Resource, VkDescriptorType ResourceType, int BindingSet, int Binding, int DescriptorCount>
    struct ShaderBinding
    {
        using infoType = Resource*;
        static constexpr VkDescriptorType type() { return ResourceType; }

        static constexpr int get_binding_set() { return BindingSet; }
        static constexpr int get_binding() { return Binding; }
        static constexpr int get_descriptor_count() { return DescriptorCount; }
    };

    template <typename Resource, VkDescriptorType ResourceType, int Stages, int Binding, int DescriptorCount>
    struct ResourceBinding
    {
        using resourceType = Resource*;
        static constexpr int get_stages() { return Stages; }
        static constexpr VkDescriptorType type() { return ResourceType; }
        static constexpr int get_binding() { return Binding; }
        static constexpr int get_descriptor_count() { return DescriptorCount; }

        ResourceBinding(Resource* info)
        {
            for (int i = 0; i < DescriptorCount; i++)
            {
                resources[i] = &info[i];
            }
        }

        void writeAll(VkDevice device, std::vector<VkDescriptorSet> descriptorSets)
        {
            if (ResourceType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) return;
            for (int i = 0; i < DescriptorCount; i++)
            {
                for (int frame = 0; frame < descriptorSets.size(); frame++)
                write(device, descriptorSets[frame], i, frame);
            }
        }

        void write(VkDevice device, VkDescriptorSet& descriptorSet, int element, int frame)
        {
            resources[element]->writeDescriptor(device, descriptorSet, get_binding(), element, type(), frame);
        }

        Resource* resources[DescriptorCount];
    };
















    template <typename type, int binding>
    struct Bind {
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
}