#pragma once

#include "shader_group.hpp"
#include "shader_resource_set.hpp"
#include "VkZero/context.hpp"
#include <vulkan/vulkan_core.h>
#include <vector>
#include <tuple>
#include <memory>
#include <stdexcept>

namespace VkZero
{
    // Pipeline validation utilities
    template <int Stage, int SetIndex, typename ShaderBinding, typename ResourceTuple>
    struct vgp_find_invalid_binding;

    template <int Stage, int SetIndex, typename ShaderBinding>
    struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<>> {
        static constexpr bool value = true;
    };

    template <int Stage, int SetIndex, typename ShaderBinding, typename First, typename... Rest>
    struct vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<First, Rest...>> {
        static constexpr bool value =
            (ShaderBinding::get_binding() != First::get_binding() ||
            ShaderBinding::get_descriptor_count() != First::get_descriptor_count() ||
            ShaderBinding::type() != First::type() ||
            (Stage & First::get_stages()) == 0) &&
            vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, std::tuple<Rest...>>::value;
    };

    // 2. Check a ShaderBinding against all ResourceSets
    template <int Stage, int SetIndex, typename ShaderBinding, typename SetTuple>
    struct vgp_shader_invalid_resource;

    template <int Stage, int SetIndex, typename ShaderBinding>
    struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding, std::tuple<>> {
        static constexpr bool value = false;
    };

    template <int Stage, int SetIndex, typename ShaderBinding, typename FirstSet, typename... RestSets>
    struct vgp_shader_invalid_resource<Stage, SetIndex, ShaderBinding, std::tuple<FirstSet, RestSets...>> {
        static constexpr bool value =
            vgp_find_invalid_binding<Stage, SetIndex, ShaderBinding, typename FirstSet::BindingResources>::value ||
            vgp_shader_invalid_resource<Stage, SetIndex + 1, ShaderBinding, std::tuple<RestSets...>>::value;
    };

    // 3. Check all bindings in a shader
    template <int Stage, typename Sets, typename ShaderBindingsTuple>
    struct vgp_shader_invalid;

    template <int Stage, typename Sets>
    struct vgp_shader_invalid<Stage, Sets, std::tuple<>> {
        static constexpr bool value = false;
    };

    template <int Stage, typename Sets, typename FirstBinding, typename... RestBindings>
    struct vgp_shader_invalid<Stage, Sets, std::tuple<FirstBinding, RestBindings...>> {
        // Use the helper to spread Sets into vgp_shader_invalid_resource
        static constexpr bool value =
            vgp_shader_invalid_resource<Stage, 0, FirstBinding, Sets>::value ||
            vgp_shader_invalid<Stage, Sets, std::tuple<RestBindings...>>::value;
    };

    // 4. Iterate over all shaders in the ShaderGroup
    template <typename Sets, typename ShaderTuple>
    struct vgp_invalid;

    template <typename Sets>
    struct vgp_invalid<Sets, std::tuple<>> {
        static constexpr bool value = false;
    };

    template <typename Sets, typename FirstShader, typename... RestShaders>
    struct vgp_invalid<Sets, std::tuple<FirstShader, RestShaders...>> {
        static constexpr bool value =
            vgp_shader_invalid<FirstShader::get_type(), Sets, typename FirstShader::BindingsList>::value ||
            vgp_invalid<Sets, std::tuple<RestShaders...>>::value;
    };

    // ----------------------------
    // Step 2: Main struct wrapper
    // ----------------------------
    template <typename ShaderGroup, typename... ResourceSets>
    struct validate_graphics_pipeline {
        static constexpr bool value = !vgp_invalid<std::tuple<ResourceSets...>, typename ShaderGroup::shaders>::value;
    };

    template <typename ShaderGroup, typename... ShaderResourcesBindings>
    class GraphicsPipeline
    {
        static_assert(validate_graphics_pipeline<ShaderGroup, ShaderResourcesBindings...>::value, "Graphics Pipeline Invalid");

    public:

        using Attachments = ShaderGroup::Attachments;

        GraphicsPipeline(std::unique_ptr<VulkanContext>& ctx, ShaderGroup& shaderGroup, ShaderResourcesBindings&... resources) :
        m_shaderGroup(shaderGroup),
        resources(resources...),
        pipelineLayout([&](){

            std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

            (descriptorSetLayouts.push_back(resources.getLayout()), ...);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

            VkPipelineLayout layout;
            if (vkCreatePipelineLayout(ctx->getDevice(), &pipelineLayoutInfo, nullptr,
                                        &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            return layout;
        }())
        {
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            // TODO save the shader arrays, so if it goes out of scope the references are still there
            pipelineInfo.stageCount = m_shaderGroup.size();
            pipelineInfo.pStages = m_shaderGroup.data();
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.layout = pipelineLayout;
        }

        void create_pipeline(VkDevice device, VkRenderPass renderPass)
        {
            if (pipeline)
            {
                throw std::runtime_error("Cannot use pipeline in more than one renderpass!");
            }

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        VkPipelineViewportStateCreateInfo viewportState{};
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        VkPipelineMultisampleStateCreateInfo multisampling{};
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        VkPipelineDynamicStateCreateInfo dynamicState{};

        std::vector<VkDynamicState> dynamicStates;

        vertexInputInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 0;
        vertexInputInfo.vertexAttributeDescriptionCount = 0;
        pipelineInfo.pVertexInputState = &vertexInputInfo;

        inputAssembly.sType =
            VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        pipelineInfo.pInputAssemblyState = &inputAssembly;

        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.scissorCount = 1;
        pipelineInfo.pViewportState = &viewportState;

        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        pipelineInfo.pRasterizationState = &rasterizer;

        multisampling.sType =
            VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        pipelineInfo.pMultisampleState = &multisampling;

        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        
        colorBlending.sType =
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        pipelineInfo.pColorBlendState = &colorBlending;
    
        dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT,VK_DYNAMIC_STATE_SCISSOR};
        dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        dynamicState.pDynamicStates = dynamicStates.data();
        pipelineInfo.pDynamicState = &dynamicState;

            pipelineInfo.pVertexInputState = &vertexInputInfo;
            pipelineInfo.pInputAssemblyState = &inputAssembly;
            pipelineInfo.pViewportState = &viewportState;
            pipelineInfo.pRasterizationState = &rasterizer;
            pipelineInfo.pMultisampleState = &multisampling;
            pipelineInfo.pColorBlendState = &colorBlending;
            pipelineInfo.pDynamicState = &dynamicState;

            pipelineInfo.renderPass = renderPass;

            if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo,
            nullptr, &pipeline) != VK_SUCCESS) {
                throw std::runtime_error("failed to create graphics pipeline!");
            }

            // Frees all memory in shader group because it cannot be used anymore
            ShaderGroup group = std::move(m_shaderGroup);
        }

        void bindResources(VkCommandBuffer commandBuffer, int currentFrame)
        {
            std::apply([&](auto& resource){
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, 1,
                    &resource.descriptorSets[currentFrame], 0, nullptr);
            }, resources);
        }
        
        std::tuple<ShaderResourcesBindings&...> resources;
        ShaderGroup m_shaderGroup;
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline = VK_NULL_HANDLE;

        friend class PipelineManager;
    };
}
