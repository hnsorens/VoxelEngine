#include "VkZero/vk_zero.hpp"
#include "VkZero/Internal/core_internal.hpp"

namespace VkZero
{
  void VkZeroInit()
  {
    vkZero_core = new VkZeroCoreImpl_T();
  }
}
