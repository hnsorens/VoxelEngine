#pragma once

#include "VkZero/context.hpp"
#include <cstdint>
#include <cstdio>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "VkZero/fixed_string.hpp"
#include "VkZero/window.hpp"
#include "VkZero/image.hpp"

#define RAYTRACE_HEIGHT 1080
#define RAYTRACE_WIDTH 1920

#define ALIGN_UP(value, alignment)                                             \
  (((value) + (alignment) - 1) & ~((alignment) - 1))

namespace VkZero 
{
    template <FixedString Name>
    class RenderPassResource
    {
    public:
        RenderPassResource(AttachmentImage* image) : resource{image} {}

        static constexpr FixedString name = Name.value;
        AttachmentImage* resource;
    };

    namespace RenderPassResourceSetDetails
    {
        // Compile-time name matching to find resource index
        template <FixedString TargetName, typename Tuple, std::size_t Index = 0>
        constexpr std::size_t findResourceIndex()
        {
            if constexpr (Index >= std::tuple_size_v<Tuple>) {
                static_assert(Index < std::tuple_size_v<Tuple>, 
                    "Resource not found for attachment");
                return Index;
            } else {
                using Element = std::tuple_element_t<Index, Tuple>;
                if constexpr (Element::name == TargetName) {
                    return Index;
                } else {
                    return findResourceIndex<TargetName, Tuple, Index + 1>();
                }
            }
        }
    };

    template <typename... Resources>
    class RenderPassResourceSet
    {
    public:
        RenderPassResourceSet(Resources... resources) : images{resources...} {}
        int framebufferCount = 3;

        template <typename Attachments>
        std::vector<VkImageView> getAttachments(int index)
        {
            std::vector<VkImageView> attachments;
            attachments.reserve(std::tuple_size_v<Attachments>);
            
            // For each attachment, find the matching resource at compile time
            [&] <std::size_t... I> (std::index_sequence<I...>) {
                (attachments.push_back(
                    std::get<
                        RenderPassResourceSetDetails::findResourceIndex<
                            std::tuple_element_t<I, Attachments>::name,
                            std::tuple<Resources...>
                        >()
                    >(images).resource->imageViews[index]
                ), ...);
            }(std::make_index_sequence<std::tuple_size_v<Attachments>>{});

            return std::move(attachments);
        }

        std::tuple<Resources...> images;
    };

    template <FixedString Name, VkFormat Format, int Location>
    struct ColorAttachment
    {
        static constexpr FixedString name = Name.value;
        static constexpr VkFormat get_format() { return Format; }
        static constexpr int get_location() { return Location; } 
    };

    template <FixedString Name, VkFormat Format, int Location>
    struct DepthAttachment
    {
        static constexpr FixedString name = Name.value;
        static constexpr VkFormat get_format() { return Format; }
        static constexpr int get_location() { return Location; } 
    };

    template <FixedString Name>
    struct PreserveAttachment
    {
        static constexpr FixedString name = Name.value;
    };

    template <FixedString Name, VkFormat Format, int Location>
    struct InputAttachment
    {
        static constexpr FixedString name = Name.value;
        static constexpr VkFormat get_format() { return Format; }
        static constexpr int get_location() { return Location; } 
    };
    namespace RenderPassDetails
    {
        template <typename Pipelines>
        struct get_all_attachments;

        template <>
        struct get_all_attachments<std::tuple<>>
        {
            using value = std::tuple<>;
        };

        template <typename First, typename... Rest>
        struct get_all_attachments<std::tuple<First, Rest...>>
        {
            using value = decltype(
                std::tuple_cat(
                    std::declval<typename First::Attachments>(),
                    std::declval<typename get_all_attachments<std::tuple<Rest...>>::value>()
                )
            );
        };

        // Helper to append to a tuple
        template <typename Tuple, typename T>
        struct tuple_push_back;

        template <typename... Ts, typename T>
        struct tuple_push_back<std::tuple<Ts...>, T> {
            using type = std::tuple<Ts..., T>;
        };

        // Check if a tuple contains an attachment with same name/type (for InputAttachment, also check format)
        template <typename T, typename Tuple>
        struct contains_attachment;

        template <typename T>
        struct contains_attachment<T, std::tuple<>> : std::false_type {};

        template <typename T, typename First, typename... Rest>
        struct contains_attachment<T, std::tuple<First, Rest...>>
            : std::conditional_t<
                std::is_same_v<T, First> ||
                (T::name == First::name && std::is_same_v<T, First>) || 
                (std::is_same_v<T, InputAttachment<T::name, T::get_format(), T::get_location()>> &&
                std::is_same_v<First, InputAttachment<First::name, First::get_format(), First::get_location()>> &&
                T::get_format() == First::get_format()),
                std::true_type,
                contains_attachment<T, std::tuple<Rest...>>
            > {};

        // Meta-function to filter preserve/input attachments
        template <typename Attachments, typename Tuple>
        struct filter_preserve_input;

        template <typename Tuple>
        struct filter_preserve_input<std::tuple<>, Tuple> {
            using type = std::tuple<>;
        };

        template <typename First, typename... Rest, typename All>
        struct filter_preserve_input<std::tuple<First, Rest...>, All> {
            using tail = typename filter_preserve_input<std::tuple<Rest...>, All>::type;
            using type = std::conditional_t<
                std::is_same_v<First, PreserveAttachment<First::name>> || 
                std::is_same_v<First, InputAttachment<First::name, First::get_format(), First::get_location()>>,
                std::conditional_t<contains_attachment<First, All>::value,
                                typename tuple_push_back<tail, First>::type,
                                tail>,
                typename tuple_push_back<tail, First>::type
            >;
        };

        // Assign global locations
        template <typename Tuple, int Start = 0>
        struct assign_global_locations;

        template <int Start>
        struct assign_global_locations<std::tuple<>, Start> {
            using type = std::tuple<>;
        };

        template <typename First, typename... Rest, int Start>
        struct assign_global_locations<std::tuple<First, Rest...>, Start> {
            // Produce new type with updated location
            using new_first = First;
            template <typename T> struct update_location { using type = T; };

            template <FixedString N, VkFormat F, int L>
            struct update_location<ColorAttachment<N, F, L>> { 
                using type = ColorAttachment<N, F, Start>; 
            };

            template <FixedString N, VkFormat F, int L>
            struct update_location<DepthAttachment<N, F, L>> { 
                using type = DepthAttachment<N, F, Start>; 
            };

            template <FixedString N, VkFormat F, int L>
            struct update_location<InputAttachment<N, F, L>> {
                using type = InputAttachment<N, F, Start>;
            };

            using type = decltype(std::tuple_cat(
                std::tuple<typename update_location<First>::type>{},
                typename assign_global_locations<std::tuple<Rest...>, Start + 1>::type{}
            ));
        };

        // The main meta-function
        template <typename AllAttachments>
        struct get_common_attachments
        {
            // Filter preserves/inputs
            using filtered = typename filter_preserve_input<AllAttachments, AllAttachments>::type;

            // Assign global locations
            using value = typename assign_global_locations<filtered>::type;
        };
    };

    // Compile-time tuple iteration
    template <typename Tuple, typename Func, std::size_t... I>
    constexpr void tuple_for_each_impl(Tuple&& t, Func&& f, std::index_sequence<I...>) {
        (f(std::get<I>(t)), ...);
    }

    template <typename Tuple, typename Func>
    constexpr void tuple_for_each(Tuple&& t, Func&& f) {
        tuple_for_each_impl(
            std::forward<Tuple>(t),
            std::forward<Func>(f),
            std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>{}
        );
    }

    template <typename Resources, typename... Pipelines>
    class RenderPass
    {
    public:
        using allAttachments = RenderPassDetails::get_all_attachments<std::tuple<Pipelines...>>::value;
        using commonAttachments = RenderPassDetails::get_common_attachments<allAttachments>::value;

        static_assert(std::tuple_size<allAttachments>::value > 0, "RenderPass must have at least one attachment.");

        RenderPass(uint32_t width, uint32_t height, std::unique_ptr<VulkanContext>& ctx, Resources& resources, Pipelines&... pipelines) : width{width}, height{height}, pipelines(pipelines...)
        {
            createRenderPass(ctx, resources, pipelines...);
        }
        
        void recreateSwapchain(std::unique_ptr<VulkanContext>& vulkanContext, std::unique_ptr<WindowManager>& window) {
            // Clean up old framebuffers
            for (auto framebuffer : framebuffers) {
                vkDestroyFramebuffer(vulkanContext->getDevice(), framebuffer, nullptr);
            }
            
            // Recreate the render pass and framebuffers
            // createRenderPass(ctx, resources, pipelines...);

            // size_t numImages = window->swapchainImageCount;
            // framebuffers.resize(numImages);
            
            for (size_t i = 0; i < framebuffers.size(); i++) {
                VkImageView attachments[] = {window->getSwapChainImages().imageViews[i]};
                printf("Doibng Cr %d\n", i); fflush(stdout);
                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = 1;
                framebufferInfo.pAttachments = attachments;
                framebufferInfo.width = window->getSwapChainExtent().width;
                framebufferInfo.height = window->getSwapChainExtent().height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(vulkanContext->getDevice(), &framebufferInfo, nullptr,
                                        &framebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
            printf("Finished REcreation\n"); fflush(stdout);
        }
        
        const std::vector<VkFramebuffer>& getFramebuffers() const { return framebuffers; }
        VkRenderPass getRenderPass() const { return renderPass; }

        void record(VkCommandBuffer commandBuffer, std::unique_ptr<WindowManager>& windowManager, uint32_t currentFrame, uint32_t imageIndex)
        {
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = framebuffers[imageIndex];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = windowManager->getSwapChainExtent();
            VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
            renderPassInfo.clearValueCount = 1;
            renderPassInfo.pClearValues = &clearColor;
            vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                                VK_SUBPASS_CONTENTS_INLINE);

            std::apply([&](auto& pipeline){
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline.pipeline);
                VkViewport viewport{};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = (float)windowManager->getSwapChainExtent().width;
                viewport.height = (float)windowManager->getSwapChainExtent().height;
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
                VkRect2D scissor{};
                scissor.offset = {0, 0};
                scissor.extent = windowManager->getSwapChainExtent();
                vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
                pipeline.bindResources(commandBuffer, currentFrame);
                vkCmdDraw(commandBuffer, 6, 1, 0, 0);
            }, pipelines);
            vkCmdEndRenderPass(commandBuffer);
        }
        
    private:
        void createRenderPass(std::unique_ptr<VulkanContext>& ctx, Resources& resources, Pipelines&... pipelines) {
            // Gather all global attachments
            std::vector<VkAttachmentDescription> attachmentDescriptions;
            tuple_for_each(commonAttachments{}, [&](auto att){
                using AttType = decltype(att);
                VkAttachmentDescription desc{};
                desc.samples = VK_SAMPLE_COUNT_1_BIT;
                desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
                desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
                desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
                desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
                desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

                if constexpr (std::is_same_v<AttType, ColorAttachment<AttType::name, AttType::get_format(), AttType::get_location()>>) {
                    desc.format = AttType::get_format();
                    desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
                } else if constexpr (std::is_same_v<AttType, DepthAttachment<AttType::name, AttType::get_format(), AttType::get_location()>>) {
                    desc.format = AttType::get_format();
                    desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                } else if constexpr (std::is_same_v<AttType, InputAttachment<AttType::name, AttType::get_format(), AttType::get_location()>>) {
                    desc.format = AttType::get_format();
                    desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                } else {
                    return; // PreserveAttachment is not an actual attachment
                }

                attachmentDescriptions.push_back(desc);
            });

            std::vector<std::vector<VkAttachmentReference>> attachmentRefsMap;
            std::vector<std::vector<uint32_t>> preserveRefsMap;
            // Build subpasses
            std::vector<VkSubpassDescription> subpassDescs;
            
            ([&](auto& pipeline){
                VkSubpassDescription subpass{};
                subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

                std::vector<VkAttachmentReference> colorRefs;
                std::vector<uint32_t> preserveRefs;
                VkAttachmentReference depthRef{};
                depthRef.attachment = VK_ATTACHMENT_UNUSED;

                tuple_for_each(typename std::remove_reference_t<decltype(pipeline)>::Attachments{}, [&](auto att){
                    using AttType = decltype(att);
                    uint32_t loc = AttType::get_location();
                    if constexpr (std::is_same_v<AttType, ColorAttachment<AttType::name, AttType::get_format(), AttType::get_location()>>) {
                        colorRefs.push_back({loc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
                    } else if constexpr (std::is_same_v<AttType, DepthAttachment<AttType::name, AttType::get_format(), AttType::get_location()>>) {
                        depthRef.attachment = loc;
                        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
                    } else if constexpr (std::is_same_v<AttType, PreserveAttachment<AttType::name>>) {
                        preserveRefs.push_back(loc);
                    }
                });

                attachmentRefsMap.push_back(std::move(colorRefs));
                preserveRefsMap.push_back(std::move(preserveRefs));

                subpass.colorAttachmentCount = static_cast<uint32_t>(attachmentRefsMap.back().size());
                subpass.pColorAttachments = attachmentRefsMap.back().data();
                subpass.preserveAttachmentCount = static_cast<uint32_t>(preserveRefsMap.back().size());
                subpass.pPreserveAttachments = preserveRefsMap.back().data();
                subpass.pDepthStencilAttachment = (depthRef.attachment != VK_ATTACHMENT_UNUSED) ? &depthRef : nullptr;

                subpassDescs.push_back(subpass);

            }(pipelines), ...);

            // Dependencies for proper layout transitions
            std::vector<VkSubpassDependency> dependencies;
            
            // First dependency: External to first subpass (for initial layout transition)
            VkSubpassDependency dependency1{};
            dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
            dependency1.dstSubpass = 0;
            dependency1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency1.srcAccessMask = 0;
            dependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency1.dependencyFlags = 0;
            dependencies.push_back(dependency1);
            
            // Second dependency: Last subpass to external (for final layout transition to present)
            VkSubpassDependency dependency2{};
            dependency2.srcSubpass = 0;
            dependency2.dstSubpass = VK_SUBPASS_EXTERNAL;
            dependency2.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            dependency2.dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            dependency2.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dependency2.dstAccessMask = 0;
            dependency2.dependencyFlags = 0;
            dependencies.push_back(dependency2);

            VkRenderPassCreateInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
            renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
            renderPassInfo.pAttachments = attachmentDescriptions.data();
            renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
            renderPassInfo.pSubpasses = subpassDescs.data();
            renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
            renderPassInfo.pDependencies = dependencies.data();

            fflush(stdout);
            if (vkCreateRenderPass(ctx->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create render pass!");
            }

            // Create pipelines for each subpass
            (pipelines.create_pipeline(ctx->getDevice(), renderPass), ...);

            framebuffers.resize(resources.framebufferCount);

            for (size_t i = 0; i < resources.framebufferCount; i++) {
                // VkImageView attachments[] = {swapChainImageViews[i]};

                std::vector<VkImageView> attachments = resources.template getAttachments<commonAttachments>(i);

                VkFramebufferCreateInfo framebufferInfo{};
                framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferInfo.renderPass = renderPass;
                framebufferInfo.attachmentCount = attachments.size();
                framebufferInfo.pAttachments = attachments.data();
                framebufferInfo.width = width;
                framebufferInfo.height = height;
                framebufferInfo.layers = 1;

                if (vkCreateFramebuffer(ctx->getDevice(), &framebufferInfo, nullptr,
                                        &framebuffers[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create framebuffer!");
                }
                printf("Created Framebuffer\n");
            }
        }

    private:
        uint32_t width, height;
        VkRenderPass renderPass;
        std::vector<VkFramebuffer> framebuffers;
        std::tuple<Pipelines&...> pipelines;
        friend class PipelineManager;

    };

    template <typename... Structures>
    struct PushConstantData
    {
        PushConstantData()
        {
            size_t size = (sizeof(Structures) + ...);
            data = new char(size);
        }

        ~PushConstantData()
        {
            delete data;
        }

        template <typename T>
        T* get()
        {
            return (T*)(data + offset<T>());
        }

    private:

        template <typename T>
        constexpr size_t offset()
        {
            size_t current = 0;
            ([&](){
                if constexpr (std::is_same<T, Structures>())
                {
                    return current;
                }
                else {
                    current += sizeof(Structures);
                }
            }(), ...);

            static_assert("Type does not exist in Push Constant Data");
            return 0;
        }

        char* data;

        template <typename... RaytracingPipelines>
        friend class RaytracingRenderPass;
    };

    template <typename PushConstant, typename Pipeline>
    struct RaytracingRenderPassPipeline
    {
    public:
        RaytracingRenderPassPipeline(Pipeline& pipeline, PushConstant& pushConstants) : pipeline(pipeline), pushConstantData(pushConstants) {}
    private:
        Pipeline& pipeline;
        PushConstant& pushConstantData;

        template <typename... RaytracingPipelines>
        friend class RaytracingRenderPass;
    };

    template <typename... RaytracingPipelines>
    class RaytracingRenderPass
    {
    public:
        RaytracingRenderPass(std::unique_ptr<VulkanContext>& ctx, RaytracingPipelines... pipelines) : pipelines{pipelines...} 
        {
            vkCmdTraceRaysKHR =
                reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(
                    ctx->getDevice(), "vkCmdTraceRaysKHR"));
        }

        void record(VkCommandBuffer commandBuffer, uint32_t currentFrame, uint32_t imageIndex)
        {
            std::apply([&](auto& pipeline)
            {
                vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
                                pipeline.pipeline.pipeline);
                pipeline.pipeline.bindResources(commandBuffer, currentFrame);
                *(uint32_t*)(pipeline.pushConstantData.data) = 0;
                vkCmdPushConstants(commandBuffer, pipeline.pipeline.pipelineLayout,
                                VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 8, pipeline.pushConstantData.data);
                vkCmdTraceRaysKHR(commandBuffer, &pipeline.pipeline.raygenRegion, &pipeline.pipeline.missRegion, &pipeline.pipeline.hitRegion,
                                &pipeline.pipeline.callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);

                VkMemoryBarrier barrier = {};
                barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
                barrier.srcAccessMask =
                    VK_ACCESS_SHADER_WRITE_BIT |
                    VK_ACCESS_SHADER_READ_BIT; // Ensure writes from the first trace finish
                barrier.dstAccessMask =
                    VK_ACCESS_SHADER_READ_BIT |
                    VK_ACCESS_SHADER_WRITE_BIT; // Ensure the second trace can read them
                barrier.pNext = 0;
                vkCmdPipelineBarrier(
                commandBuffer,
                VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace
                                                            // rays execution
                VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second
                                                            // trace rays execution
                0, 1, &barrier, 0, nullptr, 0, nullptr);
                *(uint32_t*)(pipeline.pushConstantData.data) = 1;
                vkCmdPushConstants(commandBuffer, pipeline.pipeline.pipelineLayout,
                                VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4, pipeline.pushConstantData.data);
                vkCmdTraceRaysKHR(commandBuffer, &pipeline.pipeline.raygenRegion, &pipeline.pipeline.missRegion, &pipeline.pipeline.hitRegion,
                                &pipeline.pipeline.callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);
                vkCmdPipelineBarrier(
                    commandBuffer,
                    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Source: First trace
                                                                // rays execution
                    VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, // Destination: Second
                                                                // trace rays execution
                    0, 1, &barrier, 0, nullptr, 0, nullptr);
                *(uint32_t*)(pipeline.pushConstantData.data) = 2;
                vkCmdPushConstants(commandBuffer, pipeline.pipeline.pipelineLayout,
                                VK_SHADER_STAGE_RAYGEN_BIT_KHR, 0, 4, pipeline.pushConstantData.data);
                vkCmdTraceRaysKHR(commandBuffer, &pipeline.pipeline.raygenRegion, &pipeline.pipeline.missRegion, &pipeline.pipeline.hitRegion,
                                &pipeline.pipeline.callableRegion, RAYTRACE_WIDTH, RAYTRACE_HEIGHT, 1);
            }, pipelines);
        }

        PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR;
        std::tuple<RaytracingPipelines...> pipelines;
    };
}