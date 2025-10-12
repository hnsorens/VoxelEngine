#pragma once

#include "VkZero/types.hpp"
#include "VkZero/fixed_string.hpp"
#include <tuple>
#include <vector>

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


    template <DescriptorType Type>
    struct DescriptorTypeObject
    {
        static constexpr DescriptorType value = Type;
    };

    template <typename Ts>
    struct get_shader_bindings;

    // Base case: empty tuple
    template <>
    struct get_shader_bindings<std::tuple<>> {
        using type = std::tuple<>;
    };

    // Recursive case
    template <typename First, typename... Rest>
    struct get_shader_bindings<std::tuple<First, Rest...>> {
        using type = decltype(
            std::tuple_cat(
                std::declval<typename First::BindingsList>(),
                std::declval<typename get_shader_bindings<std::tuple<Rest...>>::type>()
            )
        );
    };

    template <typename T>
    struct DescriptorTypes;

    template<>
    struct DescriptorTypes<std::tuple<>> {
        using type = std::tuple<>;
    };

    template <typename First, typename... Rest>
    struct DescriptorTypes<std::tuple<First, Rest...>> {
        using type = decltype(
            std::tuple_cat(
                std::tuple<DescriptorTypeObject<First::type()>>{},
                typename DescriptorTypes<std::tuple<Rest...>>::type{}
            )
        );
    };

    template <typename T, typename Tuple>
    struct contains;

    template <typename T>
    struct contains<T, std::tuple<>> : std::false_type {};

    template <typename T, typename U, typename... Rest>
    struct contains<T, std::tuple<U, Rest...>> 
        : std::conditional_t<std::is_same_v<T, U>, std::true_type, contains<T, std::tuple<Rest...>>> {};

    template <typename Tuple>
    struct unique_types;

    template <>
    struct unique_types<std::tuple<>> { using type = std::tuple<>; };

    template <typename First, typename... Rest>
    struct unique_types<std::tuple<First, Rest...>> {
        using Tail = typename unique_types<std::tuple<Rest...>>::type;
        using type = std::conditional_t<
            contains<First, Tail>::value,
            Tail,
            decltype(std::tuple_cat(std::tuple<First>{}, Tail{}))
        >;
    };


    struct ShaderLibraryBase
    {
        ShaderLibraryBase(std::vector<DescriptorType> types);
    };

    // ShaderTypes class
    template <typename... Shaders>
    class ShaderLibrary : public ShaderLibraryBase {
    private:
        std::tuple<Shaders...> shaders;

    public:
        using types = std::tuple<Shaders...>;

        using typess = typename unique_types<typename DescriptorTypes<typename get_shader_bindings<types>::type>::type>::type;

        ShaderLibrary() : ShaderLibraryBase([](){
            std::vector<DescriptorType> descriptorTypes;
            std::apply([&]<typename... Ts>(Ts...){
                descriptorTypes = std::vector<DescriptorType>({Ts::value...});
            }, typess{});
            return std::move(descriptorTypes);
        }()), shaders(Shaders()...) {}

        template <FixedString Name>
        constexpr auto& get() {
            constexpr size_t index = shader_index<Name, Shaders...>::value;
            return std::get<index>(shaders);
        }
    };
}
