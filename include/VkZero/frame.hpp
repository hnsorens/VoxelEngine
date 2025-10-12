#pragma once

#include "VkZero/Internal/core_internal.hpp"
#include "VkZero/Internal/renderpass_internal.hpp"
#include "VkZero/Internal/window_internal.hpp"
#include "VkZero/info.hpp"
#include <iostream>
#include <vulkan/vulkan_core.h>
namespace VkZero {

using frame_t = uint32_t;

struct FrameBase {
  FrameBase(std::vector<struct RenderpassImpl_T *> renderpasses, WindowImpl_T* window);

  void draw();

  frame_t getFrame();

  struct FrameImpl_T *impl;
};

template <typename... Renderpasses> class Frame : public FrameBase {
public:
  Frame(Renderpasses &...renderpasses, WindowImpl_T* window)
      : FrameBase({(struct RenderpassImpl_T *)(renderpasses.impl)...}, window) {}
};
} // namespace VkZero
