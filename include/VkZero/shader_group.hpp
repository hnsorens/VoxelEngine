#pragma once

#include "shader.hpp"
#include <vulkan/vulkan_core.h>
#include <vector>
#include <tuple>

namespace VkZero
{
    namespace ShaderGroupDetails
    {
        template <typename... Shaders>
        struct validate_shader_bindings {
        private:
            // Combine all shaders' BindingsList tuple types into one tuple type
            using all_bindings_tuple = decltype(std::tuple_cat(
                std::declval<typename Shaders::BindingsList>()...
            ));

            // Check compatibility of two binding types
            template <typename A, typename B>
            struct compatible {
                static constexpr bool same_slot =
                    (A::get_binding_set() == B::get_binding_set()) &&
                    (A::get_binding()     == B::get_binding());

                static constexpr bool same_type  =
                    A::type() == B::type();

                static constexpr bool same_count =
                    (A::get_descriptor_count() == B::get_descriptor_count());

                static constexpr bool value = !same_slot || (same_type && same_count);
            };

            // Recursive pairwise checker
            template <typename... Ts>
            struct check_all {
                static constexpr bool value = true; // Empty pack or single element is fine
            };

            template <typename First, typename... Rest>
            struct check_all<First, Rest...> {
                static constexpr bool value =
                    ((compatible<First, Rest>::value) && ...) && // Compare First with all Rest
                    check_all<Rest...>::value;                   // Then recurse
            };

            // Turn tuple<Ts...> into check_all<Ts...>::value
            template <typename Tuple>
            struct tuple_check;

            template <typename... Ts>
            struct tuple_check<std::tuple<Ts...>> {
                static constexpr bool value = check_all<Ts...>::value;
            };

        public:
            static constexpr bool value = tuple_check<all_bindings_tuple>::value;
        };

        template <typename Attachments>
        struct collect_attachments;

        template <>
        struct collect_attachments<std::tuple<>>
        {
            using value = std::tuple<>;
        };

        template <typename First, typename... Rest>
        struct collect_attachments<std::tuple<First, Rest...>>
        {
            using value = decltype(std::tuple_cat(
                std::declval<typename First::Attachments>(),
                std::declval<typename collect_attachments<std::tuple<Rest...>>::value>() 
            ));
        };

        template <typename Shader, typename ShaderCompares>
        struct validate_push_constant_for_shader;

        template <typename Shader>
        struct validate_push_constant_for_shader<Shader, std::tuple<>>
        {
            static constexpr bool value = true;
        };

        template <typename Shader, typename First, typename... Rest>
        struct validate_push_constant_for_shader<Shader, std::tuple<First, Rest...>>
        {
            static constexpr bool value = (std::is_same<typename Shader::PushConstantType, typename First::PushConstantType>() || std::is_same<typename Shader::PushConstantType, void>() || std::is_same<typename First::PushConstantType, void>()) &&
                    validate_push_constant_for_shader<Shader, std::tuple<Rest...>>::value;
        };
    }

    template <typename T, int ShaderStages>
    struct PushConstant
    {
        using Structure = T;
        static constexpr int shaderStages = ShaderStages;
    };

    template <typename... PushConstants>
    class ShaderPushConstants
    {
    public:
        ShaderPushConstants()
        {
            
            uint32_t currentOffset = 0;
            ranges.reserve(std::tuple_size<std::tuple<PushConstants...>>());
            [&] <std::size_t... Is> (std::index_sequence<Is...>) {
                (( [&] {
                    using PushConstant = std::tuple_element_t<Is, std::tuple<PushConstants...>>;
                    VkPushConstantRange range{};
                    range.stageFlags = PushConstant::shaderStages;
                    range.offset = currentOffset;
                    range.size = sizeof(typename PushConstant::Structure);
                    
                    ranges.push_back(range);
                    currentOffset += range.size;
                }() ), ...);
            }(std::make_index_sequence<std::tuple_size_v<std::tuple<PushConstants...>>>{});
        }

    private:

        std::vector<VkPushConstantRange> ranges;

        template <typename ShaderGroup, typename... ResourceSets>
        friend class RaytracingPipeline;
        template <typename ShaderGroup, typename... ResourceSets>
        friend class GraphicsPipeline;
        template <typename ShaderPushConstants, typename... Shaders>
        friend class ShaderGroup;
    };

    template <typename ShaderPushConstants, typename... Shaders>
    class ShaderGroup
    {
        // static_assert(all_shader_types_unique<Shaders...>::value,
        //              "Shader types conflict - multiple shaders with the same shader type!");
        static_assert(ShaderGroupDetails::validate_shader_bindings<Shaders...>::value,
                    "Shader bindings conflict - same binding number with different resource type or different binding counts!");
    public:

        
        using shaders = std::tuple<Shaders...>;
        using Attachments = ShaderGroupDetails::collect_attachments<shaders>::value;
        
        // Original constructor
        ShaderGroup(ShaderPushConstants& pushConstants, Shaders&... shaders) : 
        pushConstants(pushConstants),
        m_shaders{shaders.getShaderInfo()...} 
        {}

        size_t size()
        {
            return m_shaders.size();
        }

        VkPipelineShaderStageCreateInfo* data()
        {
            return m_shaders.data();
        }
        
    private:
        ShaderPushConstants& pushConstants;
        std::vector<VkPipelineShaderStageCreateInfo> m_shaders;

        template <typename ShaderGroup, typename... ResourceSets>
        friend class RaytracingPipeline;
        template <typename ShaderGroup, typename... ResourceSets>
        friend class GraphicsPipeline;
    };
}
