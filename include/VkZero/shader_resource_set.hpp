#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/binding.hpp"
#include "VkZero/descriptor_pool.hpp"
#include "VkZero/info.hpp"
#include <vulkan/vulkan_core.h>
#include <tuple>
#include <vector>
#include <memory>
#include <stdexcept>

namespace VkZero
{
    namespace ShaderResourceSetDetails
    {
        /**
         * @brief Checks if any binding types have duplicate binding numbers within a resource set
         * @tparam Bindings... Variadic template of binding types to check
         * @return true if duplicates are found, false otherwise
         * 
         * This ensures that each binding in a resource set has a unique binding number.
         * Duplicate binding numbers would cause Vulkan descriptor set creation to fail.
         */
        template <typename... Bindings>
        struct duplicate_binding_number_checker;

        // Base case: no bindings to check
        template <>
        struct duplicate_binding_number_checker<> {
            static constexpr bool value = false;
        };

        // Recursive case: check first binding against all remaining bindings
        template <typename First, typename... Rest>
        struct duplicate_binding_number_checker<First, Rest...>
        {
            static constexpr bool value = 
                ((First::get_binding() == Rest::get_binding()) || ...) ||
                duplicate_binding_number_checker<Rest...>::value;
        };
    }

    struct ShaderResourceSetImpl_T
    {
    public:
        ShaderResourceSetImpl_T(std::vector<ResourceBindingBase*> bindings) :
        descriptorSetLayout([&]() {

            std::vector<VkDescriptorSetLayoutBinding> descriptorBindings;

            for (auto& binding : bindings)
            {
                VkDescriptorSetLayoutBinding bindInfo{};
                bindInfo.binding = binding->binding;
                bindInfo.descriptorCount = binding->descriptorCount;
                bindInfo.descriptorType = binding->type;
                bindInfo.stageFlags = binding->stages;
                bindInfo.pImmutableSamplers = nullptr;
                descriptorBindings.push_back(bindInfo);
            }

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

            for (auto& binding : bindings)
            {
                if (binding->descriptorCount > 1)
                {
                    flags.push_back(bindless_flags);
                }
                else {
                    flags.push_back(0);
                }
            }

            VkDescriptorSetLayoutBindingFlagsCreateInfoEXT extended_info{
                VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT,
                nullptr};
            extended_info.bindingCount = flags.size();
            extended_info.pBindingFlags = flags.data();

            descriptorSetLayoutInfo.pNext = &extended_info;

            VkDescriptorSetLayout layout;
            if (vkCreateDescriptorSetLayout(vkZero_core->device, &descriptorSetLayoutInfo, nullptr, &layout) != VK_SUCCESS) {
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
            if (vkAllocateDescriptorSets(vkZero_core->device, &allocInfo, descriptorSets.data())) {
                throw std::runtime_error("Failed to create raytracing descriptor set!");
            }

            return std::move(descriptorSets);
        }())
        {
            for (auto& binding : bindings)
            {
                binding->writeAll(vkZero_core->device, descriptorSets);
            }
        }

        std::vector<ResourceBindingBase*> bindings;
        VkDescriptorSetLayout descriptorSetLayout;
        std::vector<VkDescriptorSet> descriptorSets;
    };

    struct ShaderResourceSetBase
    {
    public:
        ShaderResourceSetBase(std::vector<ResourceBindingBase*> bindings)
        {
            impl = new ShaderResourceSetImpl_T(bindings);
        }

        ShaderResourceSetImpl_T* impl;
    };

    /**
     * @brief Manages a set of shader resource bindings and their corresponding Vulkan descriptor set
     * @tparam Bindings... Variadic template of binding types to include in this resource set
     * 
     * This class creates and manages a Vulkan descriptor set layout and descriptor sets
     * for a collection of shader resource bindings. It handles the creation of descriptor
     * set layouts with proper binding flags and allocates descriptor sets from the global pool.
     */
    template <typename... Bindings>
    class ShaderResourceSet : public ShaderResourceSetBase
    {
        // Ensure no duplicate binding numbers in this resource set
        static_assert(!ShaderResourceSetDetails::duplicate_binding_number_checker<Bindings...>::value, 
                     "Shader Resource Set cannot have duplicate bindings!");

    public:

        using BindingResources = std::tuple<Bindings...>;

        /**
         * @brief Constructs a ShaderResourceSet with the given bindings
         * @param bindings... The binding instances to include in this resource set
         * 
         * This constructor:
         * 1. Creates a descriptor set layout with all the bindings
         * 2. Allocates descriptor sets from the global descriptor pool
         * 3. Writes all binding data to the descriptor sets
         */
        ShaderResourceSet(Bindings&&... bindings) : ShaderResourceSetBase({&bindings...}) 
        {
        }
    };
}
