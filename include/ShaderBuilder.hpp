#include "ResourceManager.hpp"
#include "VulkanContext.hpp"
// #include "image.hpp"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>
#include <type_traits>
#include <string>
#include <vulkan/vulkan_core.h>
#include "Binding.hpp"


enum ShaderType
{
    SHADER_VERTEX = VK_SHADER_STAGE_VERTEX_BIT,
    SHADER_FRAGMENT = VK_SHADER_STAGE_FRAGMENT_BIT,
    SHADER_GEOMETRY = VK_SHADER_STAGE_GEOMETRY_BIT,
};

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
    
    ShaderPipeline(std::unique_ptr<VulkanContext>& ctx, UniqueBindingResources resources, Shaders&... shaders) : m_shaders(shaders...),
    descriptorSetLayout([&]() {
        printf("Creating descriptor set layout\n");
        fflush(stdout);
        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
        std::unordered_map<int, int> bindingStages;

        // Helper to accumulate stage flags for each binding
        auto accumulate_stages = [&](auto& shader) {
            using TypeShader = std::decay_t<decltype(shader)>;
            const ShaderType stage = TypeShader::get_type();
            
            [&]<typename... Bs>(std::tuple<Bs...>*) {  // Pointer-to-tuple avoids construction
                ([&] {
                    constexpr int binding = Bs::get_binding();  // Access static method
                    bindingStages[binding] |= static_cast<int>(stage);
                }(), ...);
            }(static_cast<typename TypeShader::BindingsList*>(nullptr));
        };
        
        // Process all shaders
        (accumulate_stages(shaders), ...);
        
        std::apply([&](auto&&... bindings) {
            // Fold expression to process each binding
            ([&](auto&& binding) {
                VkDescriptorSetLayoutBinding bindInfo{};
                bindInfo.binding = binding.get_binding();
                bindInfo.descriptorCount = binding.get_descriptor_count();
                bindInfo.descriptorType = binding.type();
                bindInfo.stageFlags = bindingStages.at(binding.get_binding());
                bindInfo.pImmutableSamplers = nullptr;
                descriptorBindings.push_back(bindInfo);
            }(bindings), ...);
        }, bindings);

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo;
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = descriptorBindings.size();
        descriptorSetLayoutInfo.pBindings = descriptorBindings.data();
        descriptorSetLayoutInfo.pNext = nullptr;

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor set layout!");
        }

        return layout;
    }()),
    descriptorPool([&](){
        printf("Creating descriptor Pool\n");
        fflush(stdout);
        std::unordered_map<VkDescriptorType, int> descriptorCounts;

        std::apply([&](auto&&... bindings) {
            // Fold expression to process each binding
            ([&](auto&& binding) {
                descriptorCounts[binding.type()] += binding.get_descriptor_count();
            }(bindings), ...);
        }, bindings);

        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& [descriptorType, descriptorCount] : descriptorCounts)
        {
            VkDescriptorPoolSize poolSize;
            poolSize.type = descriptorType;
            poolSize.descriptorCount = descriptorCount;
            poolSizes.push_back(std::move(poolSize));
        }

        VkDescriptorPoolCreateInfo createInfo;
        createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        createInfo.poolSizeCount = poolSizes.size();
        createInfo.pPoolSizes = poolSizes.data();
        createInfo.maxSets = 50;
        
        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(ctx->getDevice(), &createInfo, nullptr, &pool) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create raytracing descriptor pool!");
        }

        return pool;
    }()),
    setLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout),
    descriptorSets([&](){
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(setLayouts.size());
        allocInfo.pSetLayouts = setLayouts.data();

        std::vector<VkDescriptorSet> descriptorSets(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(ctx->getDevice(), &allocInfo, descriptorSets.data())) {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        return std::move(descriptorSets);
    }()),
    bindings([&]() {
        return std::apply([&](auto&&... resources) {
            return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
                using ResultTuple = SortedBindings;
                return ResultTuple{
                    // Ensure we're using the correct construction method
                    [&]<size_t I>(auto&& resource) {
                        using BindingType = std::tuple_element_t<I, ResultTuple>;
                        return BindingType{ctx->getDevice(), descriptorSets, resource.info};
                    }.template operator()<Is>(resources)...
                };
            }(std::make_index_sequence<sizeof...(resources)>{});
        }, std::move(resources));
    }())
    {
        std::apply([&](auto&&... bindings){
            ([&](auto&& binding){
                for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
                {
                    for (int i = 0; i < binding.get_descriptor_count(); i++)
                    binding.write(ctx->getDevice(), descriptorSets[i], i, frame);
                }
            }(bindings), ...);
        }, std::forward<SortedBindings>(bindings));
    }
    
private:
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSetLayout> setLayouts;
    std::vector<VkDescriptorSet> descriptorSets;

    SortedBindings bindings;
    VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
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