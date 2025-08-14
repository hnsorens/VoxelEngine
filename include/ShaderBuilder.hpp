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
#include "DescriptorPool.hpp"


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

template <FixedString ShaderName, FixedString Path, ShaderType Type, typename... Bindings>
class Shader {
private:

    static_assert(!has_duplicate_shader_bindings<Bindings...>::value, "Shader cannot have duplicate bindings");

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
class ShaderGroup
{
    // static_assert(all_shader_types_unique<Shaders...>::value,
    //              "Shader types conflict - multiple shaders with the same shader type!");
    static_assert(validate_shader_bindings<Shaders...>::value,
                 "Shader bindings conflict - same binding number with different resource type or different binding counts!");
public:

    
    using shaders = std::tuple<Shaders...>;
    
    // Original constructor
    ShaderGroup(Shaders&... shaders) : m_shaders(shaders...) {
        printf("Creating shader group\n");
        fflush(stdout);
    }

    std::tuple<Shaders&...> m_shaders;
};











template <typename... Shaders>
struct has_duplicate_bindings;

template <>
struct has_duplicate_bindings<> {
    static constexpr bool value = false;
};

template <typename First, typename... Rest>
struct has_duplicate_bindings<First, Rest...>
{
    static constexpr bool value = 
        ((std::is_same_v<typename First::resourceType, typename Rest::resourceType> || First::get_type() == Rest::get_type() || First::get_binding() == Rest::get_binding() || First::get_descriptor_count() == Rest::get_descriptor_count()) || ...) ||
        has_duplicate_bindings<Rest...>::value;
};

template <typename... Bindings>
class ShaderResourceSet
{

    static_assert(!has_duplicate_bindings<Bindings...>::value, "Shader Resource Set cannot have duplicate bindings!");

public:

    using BindingResources = std::tuple<Bindings...>;

    ShaderResourceSet(std::unique_ptr<VulkanContext>& ctx, Bindings&&... bindings) :
    descriptorSetLayout([&]() {
        printf("Creating descriptor set layout\n");
        fflush(stdout);

        std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;

        // Fold expression over Bindings...
        (void(
            [&] {
                using BindingType = Bindings;
                VkDescriptorSetLayoutBinding bindInfo{};
                bindInfo.binding = BindingType::get_binding();
                bindInfo.descriptorCount = BindingType::get_descriptor_count();
                bindInfo.descriptorType = BindingType::type();
                bindInfo.stageFlags = BindingType::get_stages();
                bindInfo.pImmutableSamplers = nullptr;
                descriptorBindings.push_back(bindInfo);
            }()
        ), ...); // this iterates over each Binding type

        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
        descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(descriptorBindings.size());
        descriptorSetLayoutInfo.pBindings = descriptorBindings.data();
        descriptorSetLayoutInfo.pNext = nullptr;

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor set layout!");
        }

        return layout;
    }()),
    descriptorSets([&](){
        printf("Creating descriptor sets\n");
        fflush(stdout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{MAX_FRAMES_IN_FLIGHT, descriptorSetLayout};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = DescriptorPool::instance().get();
        allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
        allocInfo.pSetLayouts = descriptorSetLayouts.data();

        std::vector<VkDescriptorSet> descriptorSets(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(ctx->getDevice(), &allocInfo, descriptorSets.data())) {
            throw std::runtime_error("Failed to create raytracing descriptor set!");
        }

        return std::move(descriptorSets);
    }()),
    bindings(
        ([&](auto&& b) {
            printf("Creating bindings\n");
        fflush(stdout);
            b.writeAll(ctx->getDevice(), descriptorSets);
            return std::move(b);
        }(std::forward<Bindings>(bindings)))...
    )
    {}

private:

    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    std::tuple<Bindings...> bindings;
};



template <int Stage, int SetIndex, typename ShaderBinding, typename ResourceTuple>
struct vgp_find_invalid_binding;

template <int Stage, int SetIndex, typename ShaderBinding>
struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<>> {
    static constexpr bool value = false;
};

template <int Stage, int SetIndex, typename ShaderBinding, typename First, typename... Rest>
struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<First, Rest...>> {
    static constexpr bool value =
        (ShaderBinding::get_binding() != First::get_binding() ||
         ShaderBinding::get_descriptor_count() != First::get_descriptor_count() ||
         ShaderBinding::type() != First::type()) ||
         (Stage & First::get_stages()) == 0 ||
        vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<Rest...>>::value;
};


// 2. Check a ShaderBinding against all ResourceSets
template <int Stage, int SetIndex, typename ShaderBinding, typename... Sets>
struct vgp_shader_invalid_resource;

template <int Stage, int SetIndex, typename ShaderBinding>
struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding> {
    static constexpr bool value = false;
};

template <int Stage, int SetIndex, typename ShaderBinding, typename FirstSet, typename... RestSets>
struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding, FirstSet, RestSets...> {
    static constexpr bool value =
        vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, typename FirstSet::BindingResources>::value ||
        vgp_shader_invalid_resource<Stage, SetIndex + 1, ShaderBinding, RestSets...>::value;
};


// 3. Check all bindings in a shader
template <int Stage, typename ShaderBindingsTuple, typename... Sets>
struct vgp_shader_invalid;

template <int Stage, typename... Sets>
struct vgp_shader_invalid<Stage, std::tuple<>, Sets...> {
    static constexpr bool value = false;
};

template <int Stage, typename FirstBinding, typename... RestBindings, typename... Sets>
struct vgp_shader_invalid<Stage, std::tuple<FirstBinding, RestBindings...>, Sets...> {
    static constexpr bool value =
        vgp_shader_invalid_resource<Stage, 0, FirstBinding, Sets...>::value ||
        vgp_shader_invalid<Stage, std::tuple<RestBindings...>, Sets...>::value;
};


// 4. Iterate over all shaders in the ShaderGroup
template <typename ShaderTuple, typename... Sets>
struct vgp_invalid;

template <typename... Sets>
struct vgp_invalid<std::tuple<>, Sets...> {
    static constexpr bool value = false;
};

template <typename FirstShader, typename... RestShaders, typename... Sets>
struct vgp_invalid<std::tuple<FirstShader, RestShaders...>, Sets...> {
    static constexpr bool value =
        vgp_shader_invalid<FirstShader::get_type(), typename FirstShader::BindingsList, Sets...>::value ||
        vgp_invalid<std::tuple<RestShaders...>, Sets...>::value;
};


// ----------------------------
// Step 2: Main struct wrapper
// ----------------------------
template <typename ShaderGroup, typename... ResourceSets>
struct validate_graphics_pipeline {
    static constexpr bool value = !vgp_invalid<typename ShaderGroup::shaders, ResourceSets...>::value;
};


template <typename ShaderGroup, typename... ShaderResourcesBindings>
class GraphicsPipeline
{
    static_assert(validate_graphics_pipeline<ShaderGroup, ShaderResourcesBindings...>::value, "Graphics Pipeline Invalid");

public:
    GraphicsPipeline(ShaderGroup shaderGroup, ShaderResourcesBindings... resources)
    {
        printf("Creating Pipeline\n");
        fflush(stdout);
    }
};






// template <typename ShaderResource>
// class ShaderResources;

// template <typename... Shaders>
// class ShaderResourceLayout {
//     static_assert(all_shader_types_unique<Shaders...>::value,
//                  "Shader types conflict - multiple shaders with the same shader type");
//     static_assert(validate_shader_bindings<Shaders...>::value,
//                  "Shader bindings conflict - same binding number with different resource type");
    
// public:
//     using UniqueBindings = typename CombinedBindings<Shaders...>::type;
//     using SortedBindings = typename SortBindings<UniqueBindings>::type;
//     using UniqueBindingResources = typename BindingResources<SortedBindings>::type;
    
//     ShaderResourceLayout(std::unique_ptr<VulkanContext>& ctx, Shaders&... shaders) : m_shaders(shaders...),
//     descriptorSetLayout([&]() {
//         printf("Creating descriptor set layout\n");
//         fflush(stdout);
//         std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;
//         std::unordered_map<int, int> bindingStages;

//         // Helper to accumulate stage flags for each binding
//         auto accumulate_stages = [&](auto& shader) {
//             using TypeShader = std::decay_t<decltype(shader)>;
//             const ShaderType stage = TypeShader::get_type();
            
//             [&]<typename... Bs>(std::tuple<Bs...>*) {  // Pointer-to-tuple avoids construction
//                 ([&] {
//                     constexpr int binding = Bs::get_binding();  // Access static method
//                     bindingStages[binding] |= static_cast<int>(stage);
//                 }(), ...);
//             }(static_cast<typename TypeShader::BindingsList*>(nullptr));
//         };
        
//         // Process all shaders
//         (accumulate_stages(shaders), ...);
        
//         std::make_index_sequence<std::tuple_size_v<SortedBindings>> seq;

//         [&]<std::size_t... Is>(std::index_sequence<Is...>) {
//             // Fold over the indices
//             ([&] {
//                 using BindingType = std::tuple_element_t<Is, SortedBindings>;
//                 VkDescriptorSetLayoutBinding bindInfo{};
//                 bindInfo.binding = BindingType::get_binding();
//                 bindInfo.descriptorCount = BindingType::get_descriptor_count();
//                 bindInfo.descriptorType = BindingType::type();
//                 bindInfo.stageFlags = bindingStages.at(BindingType::get_binding());
//                 bindInfo.pImmutableSamplers = nullptr;
//                 descriptorBindings.push_back(bindInfo);
//             }(), ...);
//         }(std::make_index_sequence<std::tuple_size_v<SortedBindings>>{});

//         VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo = {};
//         descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
//         descriptorSetLayoutInfo.bindingCount = descriptorBindings.size();
//         descriptorSetLayoutInfo.pBindings = descriptorBindings.data();
//         descriptorSetLayoutInfo.pNext = nullptr;

//         VkDescriptorSetLayout layout;
//         if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS)
//         {
//             throw std::runtime_error("failed to create raytracing descriptor set layout!");
//         }

//         return layout;
//     }()),
//     setLayouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout)
//     {}
    
// private:
//     VkDescriptorSetLayout descriptorSetLayout;
//     std::vector<VkDescriptorSetLayout> setLayouts;
    
//     VkDescriptorSetLayoutCreateInfo layoutCreateInfo;
//     std::tuple<Shaders&...> m_shaders;

//     friend ShaderResources<ShaderResourceLayout<Shaders...>>;
// };

// template <typename ShaderResource>
// class ShaderResources
// {
// public:
//     ShaderResources(std::unique_ptr<VulkanContext>& ctx, ShaderResource& shaderResourceLayout, ShaderResource::UniqueBindingResources resources) :
//     descriptorSets([&](){
//         VkDescriptorSetAllocateInfo allocInfo = {};
//         allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
//         allocInfo.descriptorPool = DescriptorPool::instance().get();
//         allocInfo.descriptorSetCount = static_cast<uint32_t>(shaderResourceLayout.setLayouts.size());
//         allocInfo.pSetLayouts = shaderResourceLayout.setLayouts.data();

//         std::vector<VkDescriptorSet> descriptorSets(MAX_FRAMES_IN_FLIGHT);
//         if (vkAllocateDescriptorSets(ctx->getDevice(), &allocInfo, descriptorSets.data())) {
//             throw std::runtime_error("Failed to create raytracing descriptor set!");
//         }

//         return std::move(descriptorSets);
//     }()),
//     bindings([&]() {
//         return std::apply([&](auto&&... resources) {
//             return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
//                 using ResultTuple = ShaderResource::SortedBindings;
//                 return ResultTuple{
//                     // Ensure we're using the correct construction method
//                     [&]<size_t I>(auto&& resource) {
//                         using BindingType = std::tuple_element_t<I, ResultTuple>;
//                         return BindingType{ctx->getDevice(), descriptorSets, resource.info};
//                     }.template operator()<Is>(resources)...
//                 };
//             }(std::make_index_sequence<sizeof...(resources)>{});
//         }, std::move(resources));
//     }())
//     {
//         std::apply([&](auto&&... bindings){
//             ([&](auto&& binding){
//                 for (int frame = 0; frame < MAX_FRAMES_IN_FLIGHT; frame++)
//                 {
//                     for (int i = 0; i < binding.get_descriptor_count(); i++)
//                     binding.write(ctx->getDevice(), descriptorSets[i], i, frame);
//                 }
//             }(bindings), ...);
//         }, std::forward<typename ShaderResource::SortedBindings>(bindings));
//     }

//     std::vector<VkDescriptorSet> descriptorSets;
//     ShaderResource::SortedBindings bindings;
// };

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