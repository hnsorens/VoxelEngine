#pragma once

#include "shader_group.hpp"
#include "shader_resource_set.hpp"
#include <vulkan/vulkan_core.h>
#include <vector>
#include <tuple>
#include <memory>
#include <stdexcept>

namespace VkZero
{
    namespace PipelineDetails
    {

        template <int Stage, typename ShaderBinding, typename Resource>
        constexpr bool resource_matches() {
            return ShaderBinding::get_binding() == Resource::get_binding() &&
                ShaderBinding::get_descriptor_count() == Resource::get_descriptor_count() &&
                ShaderBinding::type() == Resource::type() &&
                (Stage & Resource::get_stages()) != 0;
        }
    
        template <int Stage, typename ShaderBinding, typename ResourceTuple>
        constexpr bool is_binding_valid() {
            return []<typename... Resources>(std::type_identity<std::tuple<Resources...>>) {
                return (resource_matches<Stage, ShaderBinding, Resources>() || ...);
            }(std::type_identity<ResourceTuple>{});
        }
    
        template <int Stage, int SetIndex, typename ShaderBinding, typename SetTuple>
        constexpr bool cross_set_binding_validity_checker() {
            return []<typename... Sets>(std::type_identity<std::tuple<Sets...>>) {
                return (is_binding_valid<Stage, ShaderBinding, typename Sets::BindingResources>() && ...);
            }(std::type_identity<SetTuple>{});
        }
    
        template <int Stage, typename Sets, typename ShaderBindingsTuple>
        constexpr bool shader_binding_validity_checker() {
            return []<typename... ShaderBindings>(std::type_identity<std::tuple<ShaderBindings...>>) {
                return !(cross_set_binding_validity_checker<Stage, 0, ShaderBindings, Sets>() && ...);
            }(std::type_identity<ShaderBindingsTuple>{});
        }
    
        template <typename Sets, typename ShaderTuple>
        constexpr bool shader_group_validity_checker() {
            return []<typename... Shaders>(std::type_identity<std::tuple<Shaders...>>) {
                return !(shader_binding_validity_checker<Shaders::get_type(), Sets, typename Shaders::BindingsList>() && ...);
            }(std::type_identity<ShaderTuple>{});
        }
        template <typename ShaderGroup, typename... ResourceSets>
        concept graphics_pipeline_validator = shader_group_validity_checker<std::tuple<ResourceSets...>, typename ShaderGroup::shaders>();
    }

    struct GraphicsPipelineImpl
    {

    };

    template <typename ShaderGroup, typename... ShaderResourcesBindings>
    class GraphicsPipeline
    {
        // Validate that the pipeline configuration is valid at compile time
        static_assert(PipelineDetails::graphics_pipeline_validator<ShaderGroup, ShaderResourcesBindings...>, "Graphics Pipeline Invalid");

    public:

        using Attachments = ShaderGroup::Attachments;

        /**
         * @brief Constructs a graphics pipeline
         * @param ctx Vulkan context for device access
         * @param shaderGroup The shader group containing all shaders for this pipeline
         * @param resources... The resource sets to bind to this pipeline
         * 
         * This constructor creates the pipeline layout and prepares the pipeline
         * for creation. The actual pipeline is created when create_pipeline() is called.
         */
        GraphicsPipeline(ShaderGroup& shaderGroup, ShaderResourcesBindings&... resources) :
        m_shaderGroup(shaderGroup),
        resources(resources...),
        pipelineLayout([&](){

            std::vector<VkDescriptorSetLayout> descriptorSetLayouts;

            (descriptorSetLayouts.push_back(resources.impl->descriptorSetLayout), ...);

            VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
            pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();
            pipelineLayoutInfo.pushConstantRangeCount = 0;
            pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();

            VkPipelineLayout layout;
            if (vkCreatePipelineLayout(vkZero_core->device, &pipelineLayoutInfo, nullptr,
                                        &layout) != VK_SUCCESS) {
                throw std::runtime_error("failed to create pipeline layout!");
            }

            return layout;
        }())
        {
            pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            // TODO save the shader arrays, so if it goes out of scope the references are still there
            pipelineInfo.stageCount = m_shaderGroup.m_shaders.size();
            pipelineInfo.pStages = m_shaderGroup.m_shaders.data();
            pipelineInfo.subpass = 0;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
            pipelineInfo.layout = pipelineLayout;
        }

        /**
         * @brief Creates the actual Vulkan graphics pipeline
         * @param device The Vulkan device
         * @param renderPass The render pass this pipeline will be used with
         * 
         * This method sets up all the pipeline state (vertex input, rasterization,
         * color blending, etc.) and creates the final Vulkan pipeline object.
         * Can only be called once per pipeline instance.
         */
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
        }

        /**
         * @brief Binds the pipeline's resource sets to a command buffer
         * @param commandBuffer The command buffer to bind resources to
         * @param currentFrame The current frame index for double/triple buffering
         */
        void bindResources(VkCommandBuffer commandBuffer, int currentFrame)
        {
            std::apply([&](auto& resource){
                vkCmdBindDescriptorSets(
                    commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipelineLayout, 0, 1,
                    &resource.impl->descriptorSets[currentFrame], 0, nullptr);
            }, resources);
        }
        
        std::tuple<ShaderResourcesBindings&...> resources;
        ShaderGroupImpl m_shaderGroup;
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline = VK_NULL_HANDLE;

        friend class PipelineManager;
    };
}
