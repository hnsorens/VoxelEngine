#pragma once

#include "shader_types.hpp"
#include "shader.hpp"
#include "VkZero/context.hpp"
#include "VkZero/fixed_string.hpp"
#include <tuple>
#include <memory>

namespace VkZero
{
    // Shader index finder
    template <FixedString Name, typename... Shaders>
    struct shader_index;

    // Base case (not found)
    template <FixedString Name>
    struct shader_index<Name> {
        static_assert("Shader not found in registry");
        static constexpr int value = -1;
    };

    // Recursive case
    template <FixedString Name, typename First, typename... Rest>
    struct shader_index<Name, First, Rest...> {
    private:
        static constexpr bool matches = []() {
            constexpr FixedString first_name = First::name;
            return first_name == Name;
        }();
        
    public:
        static constexpr int value = matches ? 0 : (1 + shader_index<Name, Rest...>::value);
    };

    // ShaderTypes class
    template <typename... Shaders>
    class ShaderTypes {
    private:
        std::tuple<Shaders...> shaders;

    public:
        using types = std::tuple<Shaders...>;

        ShaderTypes(std::unique_ptr<VulkanContext>& ctx) : shaders(Shaders(ctx)...) {}

        template <FixedString Name>
        constexpr auto& get() {
            constexpr size_t index = shader_index<Name, Shaders...>::value;
            return std::get<index>(shaders);
        }
    };
}
