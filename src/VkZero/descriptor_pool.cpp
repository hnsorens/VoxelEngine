#include "VkZero/Internal/descriptor_pool_internal.hpp"
#include "VkZero/Internal/core_internal.hpp"
#include "shaders.hpp"
#include <tuple>
#include <utility>
#include <vulkan/vulkan_core.h>
#include "Engine.hpp"

using namespace VkZero;

#define MAX_POOL_SETS 1000
#define MAX_DESCRIPTOR_COUNT 2000

using namespace VkZero;

template <VkDescriptorType Type>
struct DescriptorTypeObject
{
    static constexpr VkDescriptorType value = Type;
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
            std::tuple<DescriptorTypeObject<(VkDescriptorType)First::type()>>{},
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

DescriptorPool::DescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes;


    using types = typename unique_types<DescriptorTypes<get_shader_bindings<GlobalShaderLibrary::types>::type>::type>::type;

    std::apply([&]<typename... Ts>(Ts...){
       ([&](VkDescriptorType type){
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = MAX_DESCRIPTOR_COUNT;
        poolSizes.push_back(poolSize);
       }(Ts::value), ...);
    }, types{});

    VkDescriptorPoolCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.maxSets = MAX_POOL_SETS;
    createInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

    if (vkCreateDescriptorPool(vkZero_core->device, &createInfo, nullptr, &pool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create raytracing descriptor pool!");
    }
}