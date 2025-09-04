#pragma once
namespace VkZero
{
    class BindResource
    {
        virtual void writeDescriptor(void* descriptorWrite, int frame) = 0;
    };
}