#include "VkZero/Internal/renderpass_internal.hpp"

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

RenderPassBase::RenderPassBase(uint32_t width, uint32_t height,
               RenderPassResourceSetImpl_T *resources,
               std::vector<GraphicsPipelineImpl_T *> pipelines,
               std::vector<AttachmentImpl_T *> requiredAttachments) {
  impl = new RenderPassImpl_T(width, height, resources, pipelines,
                              requiredAttachments);
}

PushConstantDataBase::PushConstantDataBase(size_t size)
{
  impl = new PushConstantDataImpl_T(size);
}

RaytracingRenderpassBase::RaytracingRenderpassBase(
      std::vector<
          std::pair<RaytracingPipelineImpl_T *, PushConstantDataImpl_T *>>
          pipelines) {
    impl = new RaytracingRenderpassImpl_T(pipelines);
  }
