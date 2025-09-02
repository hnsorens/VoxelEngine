#pragma once

#include "shader_types.hpp"
#include "shader_bindings.hpp"
#include "VkZero/resource_manager.hpp"
#include "VkZero/context.hpp"
#include "VkZero/fixed_string.hpp"
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>
#include <type_traits>
#include <string>
#include <vulkan/vulkan_core.h>

namespace VkZero
{
    namespace ShaderDetails
    {
        template <typename... Shaders>
        struct has_duplicate_shader_bindings;

        template <>
        struct has_duplicate_shader_bindings<> {
            static constexpr bool value = false;
        };

        template <typename First, typename... Rest>
        struct has_duplicate_shader_bindings<First, Rest...>
        {
            static constexpr bool value = 
                ((First::get_binding_set() == Rest::get_binding_set() || First::get_binding() == Rest::get_binding() || First::get_binding_count() == Rest::get_binding_count()) || ...) ||
                has_duplicate_shader_bindings<Rest...>::value;
        };

        template <typename options>
        struct find_bindings;

        template <>
        struct find_bindings<std::tuple<>>
        {
            using type = ShaderBindings<>;
        };

        // Recursive case: check first element
        template <typename First, typename... Rest>
        struct find_bindings<std::tuple<First, Rest...>> {
        private:
            // Helper: true if First is a ShaderBindings<...>   
            template <typename T>
            struct is_shader_bindings : std::false_type {};

            template <typename... Bs>
            struct is_shader_bindings<ShaderBindings<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_shader_bindings<First>::value,
                First,
                typename find_bindings<std::tuple<Rest...>>::type
            >;
        };

        template <typename options>
        struct find_attachments;

        template <>
        struct find_attachments<std::tuple<>>
        {
            using type = ShaderAttachments<>;
        };

        template <typename First, typename... Rest>
        struct find_attachments<std::tuple<First, Rest...>> {
        private:
            // Helper: true if First is a ShaderBindings<...>
            template <typename T>
            struct is_shader_attachment : std::false_type {};

            template <typename... Bs>
            struct is_shader_attachment<ShaderAttachments<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_shader_attachment<First>::value,
                First,
                typename find_attachments<std::tuple<Rest...>>::type
            >;
        };

        template <typename options>
        struct find_push_constant;

        template <>
        struct find_push_constant<std::tuple<>>
        {
            using type = ShaderPushConstant<void>;
        };

        template <typename First, typename... Rest>
        struct find_push_constant<std::tuple<First, Rest...>> {
        private:
            // Helper: true if First is a ShaderBindings<...>   
            template <typename T>
            struct is_shader_push_constant : std::false_type {};

            template <typename... Bs>
            struct is_shader_push_constant<ShaderPushConstant<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_shader_push_constant<First>::value,
                First,
                typename find_push_constant<std::tuple<Rest...>>::type
            >;
        };
    }

    template <FixedString ShaderName, FixedString Path, ShaderType Type, typename... ShaderOptions>
    class Shader {
    public:
        using Bindings = ShaderDetails::find_bindings<std::tuple<ShaderOptions...>>::type::Options;
        using Attachments = ShaderDetails::find_attachments<std::tuple<ShaderOptions...>>::type::Options;
        using PushConstantType = ShaderDetails::find_push_constant<std::tuple<ShaderOptions...>>::type::Options;
        static constexpr FixedString name = ShaderName.value;
        static constexpr FixedString path = Path.value;

        using BindingsList = Bindings;

        Shader(std::unique_ptr<VulkanContext>& ctx) 
        {
            auto shaderCode = ResourceManager::readFile(std::string{path.value});

            shaderModule = createShaderModule(ctx->getDevice(), shaderCode);

            shaderInfo.sType =
                VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderInfo.stage = (VkShaderStageFlagBits)Type;
            shaderInfo.module = shaderModule;
            shaderInfo.pName = "main";
            shaderInfo.flags = 0;
        }

        static constexpr ShaderType get_type() { return Type; }

        VkPipelineShaderStageCreateInfo getShaderInfo() { return shaderInfo; }
        
    private:

        static_assert(std::tuple_size<Attachments>::value > 0 ? Type == SHADER_FRAGMENT : true, "Only fragment shaders can contain attachments");
        static_assert(!ShaderDetails::has_duplicate_shader_bindings<Bindings>::value, "Shader cannot have duplicate bindings");

        VkShaderModule createShaderModule(VkDevice device,
                                            const std::vector<char> &code) {
            VkShaderModuleCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            createInfo.codeSize = code.size();
            createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

            VkShaderModule shaderModule;
            if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) !=
                VK_SUCCESS) {
            throw std::runtime_error("failed to create shader module!");
            }

            return shaderModule;
        }

        VkShaderModule shaderModule;
        VkPipelineShaderStageCreateInfo shaderInfo {};
    };
}
