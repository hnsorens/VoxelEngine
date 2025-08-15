#pragma once

#include "VulkanContext.hpp"
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>
#include "FixedString.hpp"
#include "ShaderBuilder.hpp"


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

// template <typename... Pipelines>
// class RenderPass
// {
// public:

//     using allAttachments = RenderPassDetails::get_all_attachments<std::tuple<Pipelines...>>::value;

//     // static_assert(RenderPassDetails::validate_attachments<Pipelines...>::value, "Attachments are not compatible!");

//     using commonAttachments = RenderPassDetails::get_common_attachments<allAttachments>::filtered;

//     static_assert(std::tuple_size<allAttachments>::value > 0, "Only fragment shaders can contain attachments");

//     RenderPass(std::unique_ptr<VulkanContext>& ctx, Pipelines&... pipelines)
//     {
//         std::vector<std::vector<VkAttachmentReference>> colorAttachmentRefs;
//         std::vector<VkAttachmentReference> depthAttachmentRefs;
//         std::vector<std::vector<uint32_t>> preserveAttachmentRefs;
//         std::vector<VkSubpassDescription> subPasses;
//         ([&](auto& pipeline){
//             std::vector<VkAttachmentReference> colorRefs;
//             VkAttachmentReference depthRef;
//             std::vector<uint32_t> preserveRefs;

//             VkSubpassDescription subpass{};
//             subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
//             subpass.colorAttachmentCount = colorRefs.size();
//             subpass.pColorAttachments = colorRefs.data();
//             subpass.preserveAttachmentCount = preserveRefs.size();
//             subpass.pPreserveAttachments = preserveRefs.data();
//             subpass.pDepthStencilAttachment = nullptr;

//             subPasses.push_back(std::move(subpass));
//             colorAttachmentRefs.push_back(std::move(colorRefs));
//             preserveAttachmentRefs.push_back(std::move(preserveRefs));
//         }(pipelines), ...);

//         std::vector<VkAttachmentDescription> attachments;

//         VkRenderPassCreateInfo renderPassInfo{};
//         renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
//         renderPassInfo.attachmentCount = attachments.size();
//         renderPassInfo.pAttachments = attachments.data();
//         renderPassInfo.subpassCount = subPasses.size();
//         renderPassInfo.pSubpasses = subPasses.data();
//         renderPassInfo.dependencyCount = 1;
//         renderPassInfo.pDependencies = &dependency;

//         if (vkCreateRenderPass(ctx->getDevice(), &renderPassInfo, nullptr, &renderPass) !=
//             VK_SUCCESS) {
//             throw std::runtime_error("failed to create render pass!");
//         }

//         (pipelines.create_pipeline(ctx->getDevice(), renderPass), ...);
//     }

// private:
//     VkRenderPass renderPass;
// };



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

template <typename... Pipelines>
class RenderPass
{
public:
    using allAttachments = RenderPassDetails::get_all_attachments<std::tuple<Pipelines...>>::value;
    using commonAttachments = RenderPassDetails::get_common_attachments<allAttachments>::value;

    static_assert(std::tuple_size<allAttachments>::value > 0, "RenderPass must have at least one attachment.");

    RenderPass(std::unique_ptr<VulkanContext>& ctx, Pipelines... pipelines)
    {
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
                desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
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

        // Optional: dependencies
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        dependency.dependencyFlags = 0;

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescriptions.size());
        renderPassInfo.pAttachments = attachmentDescriptions.data();
        renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
        renderPassInfo.pSubpasses = subpassDescs.data();
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;

        if (vkCreateRenderPass(ctx->getDevice(), &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render pass!");
        }

        // Create pipelines for each subpass
        (pipelines.create_pipeline(ctx->getDevice(), renderPass), ...);
    }

private:
    VkRenderPass renderPass;
};