#include "VkZero/shader_library.hpp"
#include "VkZero/Internal/descriptor_pool_internal.hpp"

using namespace VkZero;

ShaderLibraryBase::ShaderLibraryBase(std::vector<DescriptorType> types)
{
    DescriptorPool::instance().build(types);
}