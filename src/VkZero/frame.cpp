#include "VkZero/Internal/frame_internal.hpp"

using namespace VkZero;

FrameImpl_T::FrameImpl_T(std::vector<RenderpassImpl_T *> renderpasses, WindowImpl_T* window)
    : renderpasses(renderpasses), window(window) {
  imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
  inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

  VkSemaphoreCreateInfo semaphoreInfo{};
  semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

  VkFenceCreateInfo fenceInfo{};
  fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
  fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    if (vkCreateSemaphore(VkZero::vkZero_core->device, &semaphoreInfo,
                          nullptr,
                          &imageAvailableSemaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(VkZero::vkZero_core->device, &semaphoreInfo,
                          nullptr,
                          &renderFinishedSemaphores[i]) != VK_SUCCESS ||
        vkCreateFence(VkZero::vkZero_core->device, &fenceInfo, nullptr,
                      &inFlightFences[i]) != VK_SUCCESS) {
      throw std::runtime_error(
          "failed to create synchronization objects for a frame!");
    }
  }

  VkCommandPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
  poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
  poolInfo.queueFamilyIndex =
      VkZero::vkZero_core->queueFamilyIndices.graphicsFamily.value();

  if (vkCreateCommandPool(VkZero::vkZero_core->device, &poolInfo, nullptr,
                          &commandPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command pool!");
  }

  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.commandPool = commandPool;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandBufferCount = (uint32_t)renderpasses.size();

  for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
    commandBuffers[i].resize(renderpasses.size());
    if (vkAllocateCommandBuffers(VkZero::vkZero_core->device, &allocInfo,
                                  commandBuffers[i].data()) != VK_SUCCESS) {
      throw std::runtime_error("failed to allocate command buffers!");
    }
  }
}

void FrameImpl_T::draw() {
  VkResult fenceResult = vkWaitForFences(
      VkZero::vkZero_core->device, 1, &inFlightFences[currentFrame], VK_TRUE,
      1000000000); // 1 second timeout

  if (fenceResult == VK_TIMEOUT) {
    std::cout << "Warning: Frame fence timeout, continuing anyway"
              << std::endl;
  }
  uint32_t imageIndex;
  if (window->nextImage(imageIndex, imageAvailableSemaphores[currentFrame])) {
    for (auto r : renderpasses) {
      r->recreateSwapchain(window);
    }
    return;
  }

  vkResetFences(VkZero::vkZero_core->device, 1, &inFlightFences[currentFrame]);

  VkCommandBufferBeginInfo beginInfo = {};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
  beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
  for (int i = 0; i < renderpasses.size(); i++) {
    vkResetCommandBuffer(commandBuffers[currentFrame][i], 0);
    if (vkBeginCommandBuffer(commandBuffers[currentFrame][i], &beginInfo) !=
        VK_SUCCESS) {
      throw std::runtime_error("failed to begin recording command buffer!");
    }

    renderpasses[i]->record(commandBuffers[currentFrame][i], window,
                            currentFrame, imageIndex);

    if (vkEndCommandBuffer(commandBuffers[currentFrame][i]) != VK_SUCCESS) {
      throw std::runtime_error("failed to record raytracing command buffer!");
    }
  }

  VkSubmitInfo submitInfo{};
  submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

  VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
  VkPipelineStageFlags waitStages[] = {
      VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
  submitInfo.waitSemaphoreCount = 1;
  submitInfo.pWaitSemaphores = waitSemaphores;
  submitInfo.pWaitDstStageMask = waitStages;
  std::vector<VkCommandBuffer> commands;

  submitInfo.commandBufferCount = commandBuffers[currentFrame].size();
  submitInfo.pCommandBuffers = commandBuffers[currentFrame].data();
  VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
  submitInfo.signalSemaphoreCount = 1;
  submitInfo.pSignalSemaphores = signalSemaphores;

  if (vkQueueSubmit(VkZero::vkZero_core->graphicsQueue, 1, &submitInfo,
                    inFlightFences[currentFrame]) != VK_SUCCESS) {
    throw std::runtime_error("failed to submit draw command buffer!");
  }

  if (window->present(imageIndex, renderFinishedSemaphores[currentFrame])) {
    for (auto r : renderpasses)
    {
      r->recreateSwapchain(window);
    }
  }

  currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

FrameBase::FrameBase(std::vector<struct RenderpassImpl_T *> renderpasses, WindowImpl_T* window) {
  impl = new FrameImpl_T(std::move(renderpasses), window);
}

void FrameBase::draw() { impl->draw(); }

frame_t FrameBase::getFrame()
{
  return impl->currentFrame;
}