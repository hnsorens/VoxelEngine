#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
#include <algorithm>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <vector>
#include <type_traits>
#include <string>
#include <vulkan/vulkan_core.h>

enum BindingType {
    BINDING_IMAGE,
    BINDING_BUFFER,
};

enum ShaderType
{
    SHADER_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
    SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
    SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
};

class Binding {
    virtual BindingType bindingType() = 0;
    virtual int binding() = 0;
};

template <BindingType ResourceType, int b, int Count>
struct BindingBase : Binding
{
    static constexpr BindingType type() { return ResourceType; }
    static constexpr int get_binding() { return b; }
    static constexpr int get_descriptor_count() {return Count; }

    BindingType bindingType() override { return type(); }
    int binding() override { return get_binding(); }
    // using type = Info;
};

template <typename T>
inline constexpr bool is_binding_v =
    std::is_base_of_v<BindingBase<T::type(), T::get_binding(), T::get_descriptor_count()>, T>;

struct ImageBindingInfo
{

};

template <int binding, int bindingCount>
struct ImageBinding : BindingBase<BINDING_IMAGE, binding, bindingCount>
{
    using infoType = ImageBindingInfo;
    ImageBinding() = default;
    ImageBinding( ImageBindingInfo info[bindingCount], int stageFlags)
    {
        // VkDescriptorSetLayoutBinding bindInfo = {};
        // bindInfo.binding = binding;
        // bindInfo.descriptorCount = bindingCount;
        // bindInfo.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        // bindInfo.stageFlags = stageFlags;
        // bindInfo.pImmutableSamplers = nullptr;
        printf("Created ImageBinding Binding: %d, BindingCount: %d, ShaderStages: %d\n", binding, bindingCount, stageFlags);
    }
};

struct BufferBindingInfo
{

};

template <int binding, int bindingCount>
struct BufferBinding : BindingBase<BINDING_BUFFER, binding, bindingCount>
{
    using infoType = BufferBindingInfo;
    BufferBinding() = default;
    BufferBinding( BufferBindingInfo info[bindingCount], int stageFlags)
    {
        printf("Created BufferBinding Binding: %d, BindingCount: %d, ShaderStages: %d\n", binding, bindingCount, stageFlags);
    }
    
};

template <int Binding, int DescriptorCount>
struct BindingSlot {
    template <typename T>
    struct Bind {
        using infoType = T;
        T info[DescriptorCount];
        
        // Add constructor
        Bind(const T (&arr)[DescriptorCount]) {
            std::copy(std::begin(arr), std::end(arr), std::begin(info));
        }
        
        // For single-element initialization
        Bind(const T& value) requires (DescriptorCount == 1) {
            info[0] = value;
        }
        
        static constexpr int get_binding() { return Binding; }
        static constexpr int get_descriptor() { return DescriptorCount; }
    };
};

template <typename type, int binding>
struct Bind {
    static_assert(is_binding_v<type>);
    using infoType = typename type::infoType;
    static constexpr BindingType get_type() { return type::type(); }
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

template <typename... Shaders>
struct has_duplicate_shader_types;

template <>
struct has_duplicate_shader_types<> {
    static constexpr bool value = false;
};

template <typename First, typename... Rest>
struct has_duplicate_shader_types<First, Rest...>
{
    static constexpr bool value = 
        ((First::get_type() == Rest::get_type()) || ...) ||
        has_duplicate_shader_types<Rest...>::value;
};

template <typename... Shaders>
struct all_shader_types_unique {
    static constexpr bool value = !has_duplicate_shader_types<Shaders...>::value;
};

// Combine bindings from multiple shaders
template <typename... Shaders>
struct CombinedBindings {
    using AllBindings = decltype(std::tuple_cat(typename Shaders::BindingsList{}...));
    
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

// Validate that matching bindings have the same type
template <typename... Shaders>
struct validate_shader_bindings {
    template <typename Tuple>
    struct check_conflicts;
    
    template <typename... Bindings>
    struct check_conflicts<std::tuple<Bindings...>> {
        template <typename Binding>
        struct check_binding {
            static constexpr bool value = []{
                constexpr int binding = Binding::get_binding();
                using CurrentType = Binding;
                
                bool valid = true;
                using ShaderList = std::tuple<Shaders...>;
                
                // Check each shader for this binding
                [&]<size_t... Is>(std::index_sequence<Is...>) {
                    ([&] {
                        using Shader = std::tuple_element_t<Is, ShaderList>;
                        if constexpr (has_binding<binding, typename Shader::BindingsList>::value) {
                            using ShaderBindingType = get_binding_type<binding, typename Shader::BindingsList>;
                            valid = valid && std::is_same_v<CurrentType, ShaderBindingType>;
                        }
                    }(), ...);
                }(std::make_index_sequence<sizeof...(Shaders)>{});
                
                return valid;
            }();
        };
        
        static constexpr bool value = (check_binding<Bindings>::value && ...);
    };
    
    static constexpr bool value = 
        check_conflicts<typename CombinedBindings<Shaders...>::type>::value;
};

template <typename Tuple>
struct BindingResources;

template <typename... Bindings>
struct BindingResources<std::tuple<Bindings...>> {
    template <typename B>
    struct BindingToSlot {
        using type = typename BindingSlot<B::get_binding(), B::get_descriptor_count()>::template Bind<typename B::infoType>;
    };

    using type = typename std::tuple<typename BindingToSlot<Bindings>::type...>;
};

template<typename A, typename B>
struct BindingLess {
    static constexpr bool value = A::get_binding() < B::get_binding();
};
// Helper to filter elements less than pivot

// Generic tuple filter
template<typename Tuple, template<typename> class Predicate>
struct FilterTuple;

template<template<typename> class Predicate>
struct FilterTuple<std::tuple<>, Predicate> {
    using type = std::tuple<>;
};

template<typename T, typename... Ts, template<typename> class Predicate>
struct FilterTuple<std::tuple<T, Ts...>, Predicate> {
    using type = decltype(std::tuple_cat(
        std::conditional_t<Predicate<T>::value, std::tuple<T>, std::tuple<>>{},
        typename FilterTuple<std::tuple<Ts...>, Predicate>::type{}
    ));
};

template<typename T, typename... Ts>
struct FilterLess {
    template<typename U>
    struct Predicate {
        static constexpr bool value = BindingLess<U, T>::value;
    };
    
    using type = typename FilterTuple<std::tuple<Ts...>, Predicate>::type;
};

// Helper to filter elements greater than or equal to pivot
template<typename T, typename... Ts>
struct FilterGreaterEqual {
    template<typename U>
    struct Predicate {
        static constexpr bool value = !BindingLess<U, T>::value;
    };
    
    using type = typename FilterTuple<std::tuple<Ts...>, Predicate>::type;
};

template <typename Tuple>
struct SortBindings;

template<>
struct SortBindings<std::tuple<>> {
    using type = std::tuple<>;
};

template<typename T>
struct SortBindings<std::tuple<T>> {
    using type = std::tuple<T>;
};

// Updated SortBindings implementation
template<typename Pivot, typename... Others>
struct SortBindings<std::tuple<Pivot, Others...>> {
    using less_partition = typename FilterLess<Pivot, Others...>::type;
    using greater_partition = typename FilterGreaterEqual<Pivot, Others...>::type;
    
    using less_sorted = typename SortBindings<less_partition>::type;
    using greater_sorted = typename SortBindings<greater_partition>::type;
    
    using type = decltype(std::tuple_cat(
        less_sorted{},
        std::tuple<Pivot>{},
        greater_sorted{}
    ));
};

template<size_t N>
struct FixedString {
    char value[N] = {};
    
    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, value);
    }
    
    // Constexpr comparison operator
    template<size_t M>
    constexpr bool operator==(const FixedString<M>& other) const {
        if (N != M) return false;
        for (size_t i = 0; i < N; ++i) {
            if (value[i] != other.value[i]) return false;
        }
        return true;
    }
};

template <FixedString ShaderName, FixedString Path, ShaderType Type, typename... Bindings>
class Shader {
public:
static constexpr FixedString name = ShaderName.value;
    static constexpr FixedString path = Path.value;

    using BindingsList = std::tuple<Bindings...>;

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
    
private:

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

template <typename... Shaders>
class ShaderPipeline {
    static_assert(all_shader_types_unique<Shaders...>::value,
                 "Shader types conflict - multiple shaders with the same shader type");
    static_assert(validate_shader_bindings<Shaders...>::value,
                 "Shader bindings conflict - same binding number with different resource type");
    
public:
    using UniqueBindings = typename CombinedBindings<Shaders...>::type;
    using SortedBindings = typename SortBindings<UniqueBindings>::type;
    using UniqueBindingResources = typename BindingResources<SortedBindings>::type;
    
    ShaderPipeline(UniqueBindingResources resources, Shaders&... shaders) : m_shaders(shaders...),
    bindings([&]() {
        
        
    std::unordered_map<int, int> bindingStages;

    // Helper to accumulate stage flags for each binding
    auto accumulate_stages = [&](auto& shader) {
        using TypeShader = std::decay_t<decltype(shader)>;
        const ShaderType stage = TypeShader::get_type();
        
        [&]<typename... Bs>(std::tuple<Bs...>) {
            ([&] {
                constexpr int binding = Bs::get_binding();
                bindingStages[binding] |= (int)stage;
            }(), ...);
        }(typename TypeShader::BindingsList{});
    };
    
    // Process all shaders
    (accumulate_stages(shaders), ...);
    return std::apply([&](auto&&... resources) {

        return [&bindingStages, &resources...]<std::size_t... Is>(std::index_sequence<Is...>) {
            using ResultTuple = SortedBindings;
            return ResultTuple{
                // Ensure we're using the correct construction method
                [&]<size_t I>(auto&& resource) {
                    using BindingType = std::tuple_element_t<I, ResultTuple>;
                    return BindingType{resource.info, bindingStages.at(BindingType::get_binding())};
                }.template operator()<Is>(resources)...
            };
        }(std::make_index_sequence<sizeof...(resources)>{});
    }, std::move(resources));
}())
    {

    }
    
private:
    SortedBindings bindings;
    std::tuple<Shaders&...> m_shaders;
};

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
    ShaderTypes(std::unique_ptr<VulkanContext>& ctx) : shaders(Shaders(ctx)...) {}

    template <FixedString Name>
    constexpr auto& get() {
        constexpr size_t index = shader_index<Name, Shaders...>::value;
        return std::get<index>(shaders);
    }
};

template<int Binding, int N, typename T>
auto make_binding_array(const T& d) {
    T arr[N];
    std::fill(std::begin(arr), std::end(arr), d);
    return typename BindingSlot<Binding, N>::template Bind<T>{arr};
}

// int main() {
//     // These will compile (compatible bindings)
//     std::unique_ptr<VulkanContext> ctx;

//     ShaderTypes<
//         Shader<
//             "main_frag",
//             "path1",
//             SHADER_VERTEX,
//             ImageBinding<1, 512>,
//             BufferBinding<2, 1>
//         >,
//         Shader<
//             "main_vert",
//             "path2",
//             SHADER_FRAGMENT,
//             ImageBinding<1, 512>,
//             BufferBinding<3, 1>
//         >
//     > shaders(ctx);

//     auto& vert = shaders.get<"maind_vert">();
//     auto& frag = shaders.get<"main_frag">();

//     auto image_bindings = make_binding_array<1, 512, ImageBindingInfo>({  });

//     // auto& vert = ShaderRegistry::get<"name1">();
    
//     ShaderPipeline pipelineBuilder({
//         BindingSlot<1, 512>::Bind{image_bindings}, 
//         BindingSlot<2, 1>::Bind{BufferBindingInfo{}}, 
//         BindingSlot<3, 1>::Bind{BufferBindingInfo{}}
//     }, vert, frag);
    
//     return 0;
// }