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
    // ============================================================================
    // Pipeline Validation Utilities
    // ============================================================================
    
    /**
     * @brief Checks if a shader binding is invalid (not found or incompatible) in a resource tuple
     * @tparam Stage The shader stage this binding is used in
     * @tparam SetIndex The descriptor set index being checked
     * @tparam ShaderBinding The shader binding type to validate
     * @tparam ResourceTuple The tuple of resource binding types to search through
     * @return true if the binding is invalid (not found or incompatible), false if valid
     * 
     * A binding is considered invalid if:
     * - It's not found in the resource tuple, OR
     * - It has a different binding number, descriptor count, or type, OR
     * - The resource doesn't support the required shader stage
     */
    template <int Stage, int SetIndex, typename ShaderBinding, typename ResourceTuple>
    struct binding_validity_checker;

    // Base case: empty resource tuple means binding is invalid (not found)
    template <int Stage, int SetIndex, typename ShaderBinding>
    struct binding_validity_checker<Stage, SetIndex, ShaderBinding, std::tuple<>> {
        static constexpr bool value = true; // Invalid because not found
    };

    // Recursive case: check first resource and continue if not found
    template <int Stage, int SetIndex, typename ShaderBinding, typename First, typename... Rest>
    struct binding_validity_checker<Stage, SetIndex, ShaderBinding, std::tuple<First, Rest...>> {
        static constexpr bool value =
            (ShaderBinding::get_binding() != First::get_binding() ||
            ShaderBinding::get_descriptor_count() != First::get_descriptor_count() ||
            ShaderBinding::type() != First::type() ||
            (Stage & First::get_stages()) == 0) &&
            binding_validity_checker<Stage, SetIndex, ShaderBinding, std::tuple<Rest...>>::value;
    };

    /**
     * @brief Checks if a shader binding is invalid across all resource sets
     * @tparam Stage The shader stage this binding is used in
     * @tparam SetIndex The current descriptor set index being checked
     * @tparam ShaderBinding The shader binding type to validate
     * @tparam SetTuple The tuple of resource sets to search through
     * @return true if the binding is invalid in all sets, false if found in any set
     */
    template <int Stage, int SetIndex, typename ShaderBinding, typename SetTuple>
    struct cross_set_binding_validity_checker;

    // Base case: no sets to check
    template <int Stage, int SetIndex, typename ShaderBinding>
    struct cross_set_binding_validity_checker<Stage, SetIndex, ShaderBinding, std::tuple<>> {
        static constexpr bool value = false; // Valid if no sets to check
    };

    // Recursive case: check current set and continue with remaining sets
    template <int Stage, int SetIndex, typename ShaderBinding, typename FirstSet, typename... RestSets>
    struct cross_set_binding_validity_checker<Stage, SetIndex, ShaderBinding, std::tuple<FirstSet, RestSets...>> {
        static constexpr bool value =
            binding_validity_checker<Stage, SetIndex, ShaderBinding, typename FirstSet::BindingResources>::value ||
            cross_set_binding_validity_checker<Stage, SetIndex + 1, ShaderBinding, std::tuple<RestSets...>>::value;
    };

    /**
     * @brief Checks if any bindings in a shader are invalid
     * @tparam Stage The shader stage being validated
     * @tparam Sets The tuple of resource sets to validate against
     * @tparam ShaderBindingsTuple The tuple of shader bindings to validate
     * @return true if any binding is invalid, false if all bindings are valid
     */
    template <int Stage, typename Sets, typename ShaderBindingsTuple>
    struct shader_binding_validity_checker;

    // Base case: no bindings to check
    template <int Stage, typename Sets>
    struct shader_binding_validity_checker<Stage, Sets, std::tuple<>> {
        static constexpr bool value = false; // Valid if no bindings to check
    };

    // Recursive case: check first binding and continue with remaining bindings
    template <int Stage, typename Sets, typename FirstBinding, typename... RestBindings>
    struct shader_binding_validity_checker<Stage, Sets, std::tuple<FirstBinding, RestBindings...>> {
        static constexpr bool value =
            cross_set_binding_validity_checker<Stage, 0, FirstBinding, Sets>::value ||
            shader_binding_validity_checker<Stage, Sets, std::tuple<RestBindings...>>::value;
    };

    /**
     * @brief Checks if any shaders in a shader group have invalid bindings
     * @tparam Sets The tuple of resource sets to validate against
     * @tparam ShaderTuple The tuple of shader types to validate
     * @return true if any shader has invalid bindings, false if all shaders are valid
     */
    template <typename Sets, typename ShaderTuple>
    struct shader_group_validity_checker;

    // Base case: no shaders to check
    template <typename Sets>
    struct shader_group_validity_checker<Sets, std::tuple<>> {
        static constexpr bool value = false; // Valid if no shaders to check
    };

    // Recursive case: check first shader and continue with remaining shaders
    template <typename Sets, typename FirstShader, typename... RestShaders>
    struct shader_group_validity_checker<Sets, std::tuple<FirstShader, RestShaders...>> {
        static constexpr bool value =
            shader_binding_validity_checker<FirstShader::get_type(), Sets, typename FirstShader::BindingsList>::value ||
            shader_group_validity_checker<Sets, std::tuple<RestShaders...>>::value;
    };

    /**
     * @brief Main validation struct for graphics pipelines
     * @tparam ShaderGroup The shader group type to validate
     * @tparam ResourceSets... The resource sets to validate against
     * @return true if the pipeline is valid, false if invalid
     * 
     * A graphics pipeline is valid if all shader bindings can be satisfied
     * by the provided resource sets.
     */
    template <typename ShaderGroup, typename... ResourceSets>
    struct graphics_pipeline_validator {
        static constexpr bool value = !shader_group_validity_checker<std::tuple<ResourceSets...>, typename ShaderGroup::shaders>::value;
    };

    /**
     * @brief Represents a Vulkan graphics pipeline with associated shaders and resources
     * @tparam ShaderGroup The shader group type for this pipeline
     * @tparam ShaderResourcesBindings... The resource sets bound to this pipeline
     * 
     * This class manages the creation and binding of a Vulkan graphics pipeline,
     * including vertex input, rasterization, color blending, and other pipeline states.
     */
    template <typename ShaderGroup, typename... ShaderResourcesBindings>
    class GraphicsPipeline
    {
        // Validate that the pipeline configuration is valid at compile time
        static_assert(graphics_pipeline_validator<ShaderGroup, ShaderResourcesBindings...>::value, "Graphics Pipeline Invalid");

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

            (descriptorSetLayouts.push_back(resources.getLayout()), ...);

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
            pipelineInfo.stageCount = m_shaderGroup.size();
            pipelineInfo.pStages = m_shaderGroup.data();
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

            // Frees all memory in shader group because it cannot be used anymore
            ShaderGroup group = std::move(m_shaderGroup);
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
