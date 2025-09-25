#pragma once

#include "VkZero/render_pass.hpp"

namespace VkZero {

struct RenderPassResourceImpl_T {
  RenderPassResourceImpl_T(const char *name, AttachmentImage *image)
      : image(image), name(name) {}

  AttachmentImage *image;
  const char *name;
};

struct RenderPassResourceSetImpl_T {
  RenderPassResourceSetImpl_T(
      std::unordered_map<std::string, RenderPassResourceImpl_T *> resources)
      : resources(std::move(resources)) {}
  int framebufferCount = 3;
  std::unordered_map<std::string, RenderPassResourceImpl_T *> resources;
};

struct AttachmentImpl_T {
  AttachmentImpl_T(const char *name, VkFormat format, int location, int type)
      : name(name), format(format), location(location), type(type) {}

  AttachmentImpl_T(const char *name) : name(name), type(3) {}

  const char *name;
  VkFormat format;
  int location;
  // TODO change back to AttachmentBase::AttachmentType
  int type;
};

struct GraphicsRenderpassImpl_T {
  GraphicsRenderpassImpl_T(uint32_t width, uint32_t height,
                   RenderPassResourceSetImpl_T *resources,
                   std::vector<GraphicsPipelineImpl_T *> pipelines,
                   std::vector<AttachmentImpl_T *> requiredAttachments);

  void createRenderPass(RenderPassResourceSetImpl_T *resources,
                        std::vector<AttachmentImpl_T *> requiredAttachments);

  void recreateSwapchain(std::unique_ptr<Window> &window);

  const std::vector<VkFramebuffer> &getFramebuffers() const;

  VkRenderPass getRenderPass() const;

  void record(VkCommandBuffer commandBuffer, std::unique_ptr<Window> &Window,
              uint32_t currentFrame, uint32_t imageIndex);

  uint32_t width, height;
  VkRenderPass renderPass;
  std::vector<VkFramebuffer> framebuffers;
  std::vector<GraphicsPipelineImpl_T *> pipelines;
};

struct PushConstantDataImpl_T {
  PushConstantDataImpl_T(size_t size)
  {
    data = new char(size);
  }

  ~PushConstantDataImpl_T() { delete data; }

  char* data;
};

} // namespace VkZero
