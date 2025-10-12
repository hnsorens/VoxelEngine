#include "VkZero/Internal/graphics_renderpass_internal.hpp"
#include "VkZero/Internal/window_internal.hpp"
#include "VkZero/Internal/graphics_pipeline_internal.hpp"
#include "VkZero/Internal/shader_group_internal.hpp"
#include "VkZero/Internal/image_internal.hpp"

using namespace VkZero;
RenderPassResourceBase::RenderPassResourceBase(const char *name,
                                               AttachmentImage *image) {
  impl = new RenderPassResourceImpl_T(name, image);
}
RenderPassResourceSetBase::RenderPassResourceSetBase(
    std::vector<RenderPassResourceImpl_T *> resources) {
  std::unordered_map<std::string, RenderPassResourceImpl_T *> resourceMap;

  for (auto resource : resources) {
    resourceMap[resource->name] = resource;
  }

  impl = new RenderPassResourceSetImpl_T(std::move(resourceMap));
}

AttachmentBase::AttachmentBase(const char *name, VkFormat format, int location,
                               AttachmentType type) {
  impl = new AttachmentImpl_T(name, format, location, (int)type);
}

AttachmentBase::AttachmentBase(const char *name) {
  impl = new AttachmentImpl_T(name);
}

GraphicsRenderpassBase::GraphicsRenderpassBase(
    uint32_t width, uint32_t height, RenderPassResourceSetImpl_T *resources,
    std::vector<GraphicsPipelineImpl_T *> pipelines,
    std::vector<AttachmentImpl_T *> requiredAttachments) {
  impl = new GraphicsRenderpassImpl_T(width, height, resources, pipelines,
                                      requiredAttachments);
}

PushConstantDataBase::PushConstantDataBase(size_t size) {
  impl = new PushConstantDataImpl_T(size);
}

GraphicsRenderpassImpl_T::GraphicsRenderpassImpl_T(
    uint32_t width, uint32_t height, RenderPassResourceSetImpl_T *resources,
    std::vector<GraphicsPipelineImpl_T *> pipelines,
    std::vector<AttachmentImpl_T *> requiredAttachments)
    : width{width}, height{height}, pipelines(pipelines) {
  createRenderPass(resources, requiredAttachments);
}

void GraphicsRenderpassImpl_T::createRenderPass(
    RenderPassResourceSetImpl_T *resources,
    std::vector<AttachmentImpl_T *> requiredAttachments) {
  // Gather all global attachments
  std::vector<VkAttachmentDescription> attachmentDescriptions;
  for (auto att : requiredAttachments) {
    VkAttachmentDescription desc{};
    desc.samples = VK_SAMPLE_COUNT_1_BIT;
    desc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    desc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    desc.format = att->format;

    switch ((AttachmentBase::AttachmentType)att->type) {
    case AttachmentBase::AttachmentType::ATTACHMENT_COLOR:
      desc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
      break;
    case AttachmentBase::AttachmentType::ATTACHMENT_DEPTH:
      desc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
      break;
    case AttachmentBase::AttachmentType::ATTACHMENT_INPUT:
      desc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      break;
    default:
      continue;
    }

    attachmentDescriptions.push_back(desc);
  }

  std::vector<std::vector<VkAttachmentReference>> attachmentRefsMap;
  std::vector<std::vector<uint32_t>> preserveRefsMap;
  // Build subpasses
  std::vector<VkSubpassDescription> subpassDescs;

  for (auto p : pipelines) {

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;

    std::vector<VkAttachmentReference> colorRefs;
    std::vector<uint32_t> preserveRefs;
    VkAttachmentReference depthRef{};
    depthRef.attachment = VK_ATTACHMENT_UNUSED;

    for (auto att : p->m_shaderGroup->attachments) {
      uint32_t loc = att->location;
      switch ((AttachmentBase::AttachmentType)att->type) {
      case AttachmentBase::AttachmentType::ATTACHMENT_COLOR:

        colorRefs.push_back({loc, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        break;
      case AttachmentBase::AttachmentType::ATTACHMENT_DEPTH:
        depthRef.attachment = loc;
        depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        break;
      case AttachmentBase::AttachmentType::ATTACHMENT_PRESERVE:
        preserveRefs.push_back(loc);
        break;
      default:
        break;
      }
    }

    attachmentRefsMap.push_back(std::move(colorRefs));
    preserveRefsMap.push_back(std::move(preserveRefs));

    subpass.colorAttachmentCount =
        static_cast<uint32_t>(attachmentRefsMap.back().size());
    subpass.pColorAttachments = attachmentRefsMap.back().data();
    subpass.preserveAttachmentCount =
        static_cast<uint32_t>(preserveRefsMap.back().size());
    subpass.pPreserveAttachments = preserveRefsMap.back().data();
    subpass.pDepthStencilAttachment =
        (depthRef.attachment != VK_ATTACHMENT_UNUSED) ? &depthRef : nullptr;

    subpassDescs.push_back(subpass);
  }

  // Dependencies for proper layout transitions
  std::vector<VkSubpassDependency> dependencies;

  // First dependency: External to first subpass (for initial layout
  // transition)
  VkSubpassDependency dependency1{};
  dependency1.srcSubpass = VK_SUBPASS_EXTERNAL;
  dependency1.dstSubpass = 0;
  dependency1.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency1.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
  dependency1.srcAccessMask = 0;
  dependency1.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
  dependency1.dependencyFlags = 0;
  dependencies.push_back(dependency1);

  // Second dependency: Last subpass to external (for final layout transition
  // to present)
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
  renderPassInfo.attachmentCount =
      static_cast<uint32_t>(attachmentDescriptions.size());
  renderPassInfo.pAttachments = attachmentDescriptions.data();
  renderPassInfo.subpassCount = static_cast<uint32_t>(subpassDescs.size());
  renderPassInfo.pSubpasses = subpassDescs.data();
  renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
  renderPassInfo.pDependencies = dependencies.data();

  fflush(stdout);
  if (vkCreateRenderPass(vkZero_core->device, &renderPassInfo, nullptr,
                         &renderPass) != VK_SUCCESS) {
    throw std::runtime_error("Failed to create render pass!");
  }

  for (auto p : pipelines) {
    p->create_pipeline(vkZero_core->device, renderPass);
  }

  framebuffers.resize(resources->framebufferCount);

  for (size_t i = 0; i < resources->framebufferCount; i++) {
    // VkImageView attachments[] = {swapChainImageViews[i]};

    std::vector<VkImageView> attachments;

    // Collect Attachments
    for (auto att : requiredAttachments) {
      attachments.push_back(
          resources->resources[att->name]->image->impl->images[i]->view);
    }

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = attachments.size();
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vkZero_core->device, &framebufferInfo, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

void GraphicsRenderpassImpl_T::recreateSwapchain(WindowImpl_T* window) {
  // Clean up old framebuffers
  for (auto framebuffer : framebuffers) {
    vkDestroyFramebuffer(vkZero_core->device, framebuffer, nullptr);
  }

  // Recreate the render pass and framebuffers
  // createRenderPass(ctx, resources, pipelines...);

  // size_t numImages = window->swapchainImageCount;
  // framebuffers.resize(numImages);
  for (size_t i = 0; i < framebuffers.size(); i++) {
    VkImageView attachments[] = {window->swapchainImages.impl->images[i]->view};
    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = attachments;
    framebufferInfo.width = window->swapChainExtent.width;
    framebufferInfo.height = window->swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(vkZero_core->device, &framebufferInfo, nullptr,
                            &framebuffers[i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to create framebuffer!");
    }
  }
}

const std::vector<VkFramebuffer> &
GraphicsRenderpassImpl_T::getFramebuffers() const {
  return framebuffers;
}
VkRenderPass GraphicsRenderpassImpl_T::getRenderPass() const {
  return renderPass;
}

void GraphicsRenderpassImpl_T::record(VkCommandBuffer commandBuffer,
                                      WindowImpl_T *window,
                                      uint32_t currentFrame,
                                      uint32_t imageIndex) {
  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = renderPass;
  renderPassInfo.framebuffer = framebuffers[imageIndex];
  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = window->swapChainExtent;
  VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
  renderPassInfo.clearValueCount = 1;
  renderPassInfo.pClearValues = &clearColor;
  vkCmdBeginRenderPass(commandBuffer, &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);

  for (auto p : pipelines) {

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      p->pipeline);
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)window->swapChainExtent.width;
    viewport.height = (float)window->swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = window->swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    p->bindResources(commandBuffer, currentFrame);
    vkCmdDraw(commandBuffer, 6, 1, 0, 0);
  }
  vkCmdEndRenderPass(commandBuffer);
}
