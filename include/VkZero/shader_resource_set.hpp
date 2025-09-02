#pragma once

#include "VkZero/context.hpp"
#include "VkZero/descriptor_pool.hpp"
#include <vulkan/vulkan_core.h>
#include <tuple>
#include <vector>
#include <memory>
#include <stdexcept>

namespace VkZero
{
    namespace ShaderResourceSetDetails
    {
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
                ((First::get_binding() == Rest::get_binding()) || ...) ||
                has_duplicate_bindings<Rest...>::value;
        };
    }

    template <typename... Bindings>
    class ShaderResourceSet
    {

        static_assert(!ShaderResourceSetDetails::has_duplicate_bindings<Bindings...>::value, "Shader Resource Set cannot have duplicate bindings!");

    public:

        using BindingResources = std::tuple<Bindings...>;

        ShaderResourceSet(std::unique_ptr<VulkanContext>& ctx, Bindings&&... bindings) :
        descriptorSetLayout([&]() {

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

            descriptorSetLayoutInfo.flags =
                VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;

            VkDescriptorBindingFlags bindless_flags =
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT |
                VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT;
            std::vector<VkDescriptorBindingFlags> flags;

            (void(
                [&] {
                    using BindingType = Bindings;

                    if (BindingType::get_descriptor_count() > 1)
                    {
                        flags.push_back(bindless_flags);
                    }
                    else {
                        flags.push_back(0);
                    }
                }()
            ), ...); // this iterates over each Binding type

            VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
                nullptr};
            extended_info.bindingCount = flags.size();
            extended_info.pBindingFlags = flags.data();

            descriptorSetLayoutInfo.pNext = &extended_info;

            VkDescriptorSetLayout layout;
            if (vkCreateDescriptorSetLayout(ctx->getDevice(), &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create descriptor set layout!");
            }

            return layout;
        }()),
        descriptorSets([&](){
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
                b.writeAll(ctx->getDevice(), descriptorSets);
                return std::move(b);
            }(std::forward<Bindings>(bindings)))...
        )
        {}

        VkDescriptorSetLayout getLayout() { return descriptorSetLayout; }

        VkDescriptorSetLayout descriptorSetLayout;
        std::vector<VkDescriptorSet> descriptorSets;
        std::tuple<Bindings...> bindings;
    };
}
