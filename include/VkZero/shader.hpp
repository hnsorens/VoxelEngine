#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "shader_types.hpp"
#include "shader_bindings.hpp"
#include "VkZero/resource_manager.hpp"
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
        /**
         * @brief Checks if any shader bindings have duplicate binding numbers, sets, or counts
         * @tparam Shaders... Variadic template of shader binding types to check
         * @return true if duplicates are found, false otherwise
         */
        template <typename... Shaders>
        struct duplicate_binding_checker;

        // Base case: no shaders to check
        template <>
        struct duplicate_binding_checker<> {
            static constexpr bool value = false;
        };

        // Recursive case: check first shader against all remaining shaders
        template <typename First, typename... Rest>
        struct duplicate_binding_checker<First, Rest...>
        {
            static constexpr bool value = 
                ((First::get_binding_set() == Rest::get_binding_set() || 
                  First::get_binding() == Rest::get_binding() || 
                  First::get_binding_count() == Rest::get_binding_count()) || ...) ||
                duplicate_binding_checker<Rest...>::value;
        };

        /**
         * @brief Searches through a tuple of shader options to find the ShaderBindings type
         * @tparam OptionsTuple The tuple of shader options to search through
         * @return The found ShaderBindings type or empty ShaderBindings if none found
         */
        template <typename OptionsTuple>
        struct bindings_extractor;

        // Base case: empty tuple
        template <>
        struct bindings_extractor<std::tuple<>>
        {
            using type = ShaderBindings<>;
        };

        // Recursive case: check first element and continue if not found
        template <typename First, typename... Rest>
        struct bindings_extractor<std::tuple<First, Rest...>> {
        private:
            /**
             * @brief Type trait to check if a type is a ShaderBindings template
             * @tparam T The type to check
             */
            template <typename T>
            struct is_bindings_type : std::false_type {};

            template <typename... Bs>
            struct is_bindings_type<ShaderBindings<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_bindings_type<First>::value,
                First,
                typename bindings_extractor<std::tuple<Rest...>>::type
            >;
        };

        /**
         * @brief Searches through a tuple of shader options to find the ShaderAttachments type
         * @tparam OptionsTuple The tuple of shader options to search through
         * @return The found ShaderAttachments type or empty ShaderAttachments if none found
         */
        template <typename OptionsTuple>
        struct attachments_extractor;

        // Base case: empty tuple
        template <>
        struct attachments_extractor<std::tuple<>>
        {
            using type = ShaderAttachments<>;
        };

        // Recursive case: check first element and continue if not found
        template <typename First, typename... Rest>
        struct attachments_extractor<std::tuple<First, Rest...>> {
        private:
            /**
             * @brief Type trait to check if a type is a ShaderAttachments template
             * @tparam T The type to check
             */
            template <typename T>
            struct is_attachments_type : std::false_type {};

            template <typename... Bs>
            struct is_attachments_type<ShaderAttachments<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_attachments_type<First>::value,
                First,
                typename attachments_extractor<std::tuple<Rest...>>::type
            >;
        };

        /**
         * @brief Searches through a tuple of shader options to find the ShaderPushConstant type
         * @tparam OptionsTuple The tuple of shader options to search through
         * @return The found ShaderPushConstant type or void ShaderPushConstant if none found
         */
        template <typename OptionsTuple>
        struct push_constant_extractor;

        // Base case: empty tuple
        template <>
        struct push_constant_extractor<std::tuple<>>
        {
            using type = ShaderPushConstant<void>;
        };

        // Recursive case: check first element and continue if not found
        template <typename First, typename... Rest>
        struct push_constant_extractor<std::tuple<First, Rest...>> {
        private:
            /**
             * @brief Type trait to check if a type is a ShaderPushConstant template
             * @tparam T The type to check
             */
            template <typename T>
            struct is_push_constant_type : std::false_type {};

            template <typename... Bs>
            struct is_push_constant_type<ShaderPushConstant<Bs...>> : std::true_type {};

        public:
            using type = std::conditional_t<
                is_push_constant_type<First>::value,
                First,
                typename push_constant_extractor<std::tuple<Rest...>>::type
            >;
        };
    }
    class ShaderBase
    {
    public:
        ShaderBase(std::string path, VkShaderStageFlagBits type);

        struct ShaderImpl_T* impl;
    };

    template <FixedString ShaderName, FixedString Path, ShaderType Type, typename... ShaderOptions>
    class Shader : public ShaderBase{
    public:
        using Bindings = ShaderDetails::bindings_extractor<std::tuple<ShaderOptions...>>::type::Options;
        using Attachments = ShaderDetails::attachments_extractor<std::tuple<ShaderOptions...>>::type::Options;
        using PushConstantType = ShaderDetails::push_constant_extractor<std::tuple<ShaderOptions...>>::type::Options;
        static constexpr FixedString name = ShaderName.value;
        static constexpr FixedString path = Path.value;

        using BindingsList = Bindings;

        Shader() : ShaderBase(std::string{path.value}, (VkShaderStageFlagBits)Type) {}
        static constexpr ShaderType get_type() { return Type; }
        
    private:

        static_assert(std::tuple_size<Attachments>::value > 0 ? Type == SHADER_FRAGMENT : true, "Only fragment shaders can contain attachments");
        static_assert(!ShaderDetails::duplicate_binding_checker<Bindings>::value, "Shader cannot have duplicate bindings");
    };
}
