#pragma once

#include "VkZero/bind_resource.hpp"
#include "VkZero/info.hpp"
#include "image.hpp"
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VkZero 
{
    enum class Format : uint32_t {
        Undefined = 0,
        R4G4UnormPack8 = 1,
        R4G4B4A4UnormPack16 = 2,
        B4G4R4A4UnormPack16 = 3,
        R5G6B5UnormPack16 = 4,
        B5G6R5UnormPack16 = 5,
        R5G5B5A1UnormPack16 = 6,
        B5G5R5A1UnormPack16 = 7,
        A1R5G5B5UnormPack16 = 8,
        R8Unorm = 9,
        R8Snorm = 10,
        R8Uscaled = 11,
        R8Sscaled = 12,
        R8Uint = 13,
        R8Sint = 14,
        R8Srgb = 15,
        R8G8Unorm = 16,
        R8G8Snorm = 17,
        R8G8Uscaled = 18,
        R8G8Sscaled = 19,
        R8G8Uint = 20,
        R8G8Sint = 21,
        R8G8Srgb = 22,
        R8G8B8Unorm = 23,
        R8G8B8Snorm = 24,
        R8G8B8Uscaled = 25,
        R8G8B8Sscaled = 26,
        R8G8B8Uint = 27,
        R8G8B8Sint = 28,
        R8G8B8Srgb = 29,
        B8G8R8Unorm = 30,
        B8G8R8Snorm = 31,
        B8G8R8Uscaled = 32,
        B8G8R8Sscaled = 33,
        B8G8R8Uint = 34,
        B8G8R8Sint = 35,
        B8G8R8Srgb = 36,
        R8G8B8A8Unorm = 37,
        R8G8B8A8Snorm = 38,
        R8G8B8A8Uscaled = 39,
        R8G8B8A8Sscaled = 40,
        R8G8B8A8Uint = 41,
        R8G8B8A8Sint = 42,
        R8G8B8A8Srgb = 43,
        B8G8R8A8Unorm = 44,
        B8G8R8A8Snorm = 45,
        B8G8R8A8Uscaled = 46,
        B8G8R8A8Sscaled = 47,
        B8G8R8A8Uint = 48,
        B8G8R8A8Sint = 49,
        B8G8R8A8Srgb = 50,
        A8B8G8R8UnormPack32 = 51,
        A8B8G8R8SnormPack32 = 52,
        A8B8G8R8UscaledPack32 = 53,
        A8B8G8R8SscaledPack32 = 54,
        A8B8G8R8UintPack32 = 55,
        A8B8G8R8SintPack32 = 56,
        A8B8G8R8SrgbPack32 = 57,
        A2R10G10B10UnormPack32 = 58,
        A2R10G10B10SnormPack32 = 59,
        A2R10G10B10UscaledPack32 = 60,
        A2R10G10B10SscaledPack32 = 61,
        A2R10G10B10UintPack32 = 62,
        A2R10G10B10SintPack32 = 63,
        A2B10G10R10UnormPack32 = 64,
        A2B10G10R10SnormPack32 = 65,
        A2B10G10R10UscaledPack32 = 66,
        A2B10G10R10SscaledPack32 = 67,
        A2B10G10R10UintPack32 = 68,
        A2B10G10R10SintPack32 = 69,
        R16Unorm = 70,
        R16Snorm = 71,
        R16Uscaled = 72,
        R16Sscaled = 73,
        R16Uint = 74,
        R16Sint = 75,
        R16Sfloat = 76,
        R16G16Unorm = 77,
        R16G16Snorm = 78,
        R16G16Uscaled = 79,
        R16G16Sscaled = 80,
        R16G16Uint = 81,
        R16G16Sint = 82,
        R16G16Sfloat = 83,
        R16G16B16Unorm = 84,
        R16G16B16Snorm = 85,
        R16G16B16Uscaled = 86,
        R16G16B16Sscaled = 87,
        R16G16B16Uint = 88,
        R16G16B16Sint = 89,
        R16G16B16Sfloat = 90,
        R16G16B16A16Unorm = 91,
        R16G16B16A16Snorm = 92,
        R16G16B16A16Uscaled = 93,
        R16G16B16A16Sscaled = 94,
        R16G16B16A16Uint = 95,
        R16G16B16A16Sint = 96,
        R16G16B16A16Sfloat = 97,
        R32Uint = 98,
        R32Sint = 99,
        R32Sfloat = 100,
        R32G32Uint = 101,
        R32G32Sint = 102,
        R32G32Sfloat = 103,
        R32G32B32Uint = 104,
        R32G32B32Sint = 105,
        R32G32B32Sfloat = 106,
        R32G32B32A32Uint = 107,
        R32G32B32A32Sint = 108,
        R32G32B32A32Sfloat = 109,
        R64Uint = 110,
        R64Sint = 111,
        R64Sfloat = 112,
        R64G64Uint = 113,
        R64G64Sint = 114,
        R64G64Sfloat = 115,
        R64G64B64Uint = 116,
        R64G64B64Sint = 117,
        R64G64B64Sfloat = 118,
        R64G64B64A64Uint = 119,
        R64G64B64A64Sint = 120,
        R64G64B64A64Sfloat = 121,
        B10G11R11UfloatPack32 = 122,
        E5B9G9R9UfloatPack32 = 123,
        D16Unorm = 124,
        X8D24UnormPack32 = 125,
        D32Sfloat = 126,
        S8Uint = 127,
        D16UnormS8Uint = 128,
        D24UnormS8Uint = 129,
        D32SfloatS8Uint = 130,
        BC1RgbUnormBlock = 131,
        BC1RgbSrgbBlock = 132,
        BC1RgbaUnormBlock = 133,
        BC1RgbaSrgbBlock = 134,
        BC2UnormBlock = 135,
        BC2SrgbBlock = 136,
        BC3UnormBlock = 137,
        BC3SrgbBlock = 138,
        BC4UnormBlock = 139,
        BC4SnormBlock = 140,
        BC5UnormBlock = 141,
        BC5SnormBlock = 142,
        BC6HUfloatBlock = 143,
        BC6HSfloatBlock = 144,
        BC7UnormBlock = 145,
        BC7SrgbBlock = 146,
        ETC2R8G8B8UnormBlock = 147,
        ETC2R8G8B8SrgbBlock = 148,
        ETC2R8G8B8A1UnormBlock = 149,
        ETC2R8G8B8A1SrgbBlock = 150,
        ETC2R8G8B8A8UnormBlock = 151,
        ETC2R8G8B8A8SrgbBlock = 152,
        EACR11UnormBlock = 153,
        EACR11SnormBlock = 154,
        EACR11G11UnormBlock = 155,
        EACR11G11SnormBlock = 156,
        ASTC4x4UnormBlock = 157,
        ASTC4x4SrgbBlock = 158,
        ASTC5x4UnormBlock = 159,
        ASTC5x4SrgbBlock = 160,
        ASTC5x5UnormBlock = 161,
        ASTC5x5SrgbBlock = 162,
        ASTC6x5UnormBlock = 163,
        ASTC6x5SrgbBlock = 164,
        ASTC6x6UnormBlock = 165,
        ASTC6x6SrgbBlock = 166,
        ASTC8x5UnormBlock = 167,
        ASTC8x5SrgbBlock = 168,
        ASTC8x6UnormBlock = 169,
        ASTC8x6SrgbBlock = 170,
        ASTC8x8UnormBlock = 171,
        ASTC8x8SrgbBlock = 172,
        ASTC10x5UnormBlock = 173,
        ASTC10x5SrgbBlock = 174,
        ASTC10x6UnormBlock = 175,
        ASTC10x6SrgbBlock = 176,
        ASTC10x8UnormBlock = 177,
        ASTC10x8SrgbBlock = 178,
        ASTC10x10UnormBlock = 179,
        ASTC10x10SrgbBlock = 180,
        ASTC12x10UnormBlock = 181,
        ASTC12x10SrgbBlock = 182,
        ASTC12x12UnormBlock = 183,
        ASTC12x12SrgbBlock = 184,
        G8B8G8R8422Unorm = 1000156000,
        B8G8R8G8422Unorm = 1000156001,
        G8B8R83Plane420Unorm = 1000156002,
        G8B8R82Plane420Unorm = 1000156003,
        G8B8R83Plane422Unorm = 1000156004,
        G8B8R82Plane422Unorm = 1000156005,
        G8B8R83Plane444Unorm = 1000156006,
        R10X6UnormPack16 = 1000156007,
        R10X6G10X6Unorm2Pack16 = 1000156008,
        R10X6G10X6B10X6A10X6Unorm4Pack16 = 1000156009,
        G10X6B10X6G10X6R10X6422Unorm4Pack16 = 1000156010,
        B10X6G10X6R10X6G10X6422Unorm4Pack16 = 1000156011,
        G10X6B10X6R10X63Plane420Unorm3Pack16 = 1000156012,
        G10X6B10X6R10X62Plane420Unorm3Pack16 = 1000156013,
        G10X6B10X6R10X63Plane422Unorm3Pack16 = 1000156014,
        G10X6B10X6R10X62Plane422Unorm3Pack16 = 1000156015,
        G10X6B10X6R10X63Plane444Unorm3Pack16 = 1000156016,
        R12X4UnormPack16 = 1000156017,
        R12X4G12X4Unorm2Pack16 = 1000156018,
        R12X4G12X4B12X4A12X4Unorm4Pack16 = 1000156019,
        G12X4B12X4G12X4R12X4422Unorm4Pack16 = 1000156020,
        B12X4G12X4R12X4G12X4422Unorm4Pack16 = 1000156021,
        G12X4B12X4R12X43Plane420Unorm3Pack16 = 1000156022,
        G12X4B12X4R12X42Plane420Unorm3Pack16 = 1000156023,
        G12X4B12X4R12X43Plane422Unorm3Pack16 = 1000156024,
        G12X4B12X4R12X42Plane422Unorm3Pack16 = 1000156025,
        G12X4B12X4R12X63Plane444Unorm3Pack16 = 1000156026,
        G16B16G16R16422Unorm = 1000156027,
        B16G16R16G16422Unorm = 1000156028,
        G16B16R163Plane420Unorm = 1000156029,
        G16B16R162Plane420Unorm = 1000156030,
        G16B16R163Plane422Unorm = 1000156031,
        G16B16R162Plane422Unorm = 1000156032,
        G16B16R163Plane444Unorm = 1000156033,
        G8B8R82Plane444Unorm = 1000330000,
        G10X6B10X6R10X62Plane444Unorm3Pack16 = 1000330001,
        G12X4B12X4R12X42Plane444Unorm3Pack16 = 1000330002,
        G16B16R162Plane444Unorm = 1000330003,
        A4R4G4B4UnormPack16 = 1000340000,
        A4B4G4R4UnormPack16 = 1000340001,
        ASTC4x4SfloatBlock = 1000066000,
        ASTC5x4SfloatBlock = 1000066001,
        ASTC5x5SfloatBlock = 1000066002,
        ASTC6x5SfloatBlock = 1000066003,
        ASTC6x6SfloatBlock = 1000066004,
        ASTC8x5SfloatBlock = 1000066005,
        ASTC8x6SfloatBlock = 1000066006,
        ASTC8x8SfloatBlock = 1000066007,
        ASTC10x5SfloatBlock = 1000066008,
        ASTC10x6SfloatBlock = 1000066009,
        ASTC10x8SfloatBlock = 1000066010,
        ASTC10x10SfloatBlock = 1000066011,
        ASTC12x10SfloatBlock = 1000066012,
        ASTC12x12SfloatBlock = 1000066013,
        A1B5G5R5UnormPack16 = 1000470000,
        A8Unorm = 1000470001,
        PVRTC12BppUnormBlockImg = 1000054000,
        PVRTC14BppUnormBlockImg = 1000054001,
        PVRTC22BppUnormBlockImg = 1000054002,
        PVRTC24BppUnormBlockImg = 1000054003,
        PVRTC12BppSrgbBlockImg = 1000054004,
        PVRTC14BppSrgbBlockImg = 1000054005,
        PVRTC22BppSrgbBlockImg = 1000054006,
        PVRTC24BppSrgbBlockImg = 1000054007,
        R16G16Sfixed5NV = 1000464000
    };

    enum class ImageLayout : uint32_t {
        Undefined = 0,
        General = 1,
        ColorAttachmentOptimal = 2,
        DepthStencilAttachmentOptimal = 3,
        DepthStencilReadOnlyOptimal = 4,
        ShaderReadOnlyOptimal = 5,
        TransferSrcOptimal = 6,
        TransferDstOptimal = 7,
        Preinitialized = 8,
        DepthReadOnlyStencilAttachmentOptimal = 1000117000,
        DepthAttachmentStencilReadOnlyOptimal = 1000117001,
        DepthAttachmentOptimal = 1000241000,
        DepthReadOnlyOptimal = 1000241001,
        StencilAttachmentOptimal = 1000241002,
        StencilReadOnlyOptimal = 1000241003,
        ReadOnlyOptimal = 1000314000,
        AttachmentOptimal = 1000314001,
        RenderingLocalRead = 1000232000,
        PresentSrc = 1000001002,
        VideoDecodeDst = 1000024000,
        VideoDecodeSrc = 1000024001,
        VideoDecodeDpb = 1000024002,
        SharedPresent = 1000111000,
        FragmentDensityMapOptimal = 1000218000,
        FragmentShadingRateAttachmentOptimal = 1000164003,
        VideoEncodeDst = 1000299000,
        VideoEncodeSrc = 1000299001,
        VideoEncodeDpb = 1000299002,
        AttachmentFeedbackLoopOptimal = 1000339000,
        VideoEncodeQuantizationMap = 1000553000
    };

    class Image
    {
    protected:

        using StagingData = struct StagingData_T*;
        using ImageData = struct ImageData_T*;
        using ImageImpl = struct ImageImpl_T*;

        Image(VkImage* vk_images, VkImageView* imageViews, size_t imageCount, ImageData* images);
        Image(uint32_t width, uint32_t height, uint32_t depth,
            Format format,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            ImageData* images, size_t imageCount);

        Image(uint32_t width, uint32_t height, uint32_t depth,
            Format format,
            VkImageUsageFlags usage,
            VkMemoryPropertyFlags properties,
            ImageData* images, StagingData* staging, size_t imageCount);

        void Write(ImageData image, StagingData staging, char* data);
        void Write(VkCommandBuffer commandBuffer, ImageData image, StagingData staging, char* data);
        void WriteDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, ImageData image);
        void changeLayout(ImageData image, VkImageLayout newLayout, int mipLevels);
        void changeLayout(VkCommandBuffer commandBuffer, ImageData image, VkImageLayout newLayout, int mipLevels);

        ImageImpl impl;
    };

    template <int MaxImageCount>
    class ImageBase : BindResource, Image
    {
    public:
        ImageBase(uint32_t width, uint32_t height, uint32_t depth,
                Format format,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties) :
                Image(width, height, depth, format, usage, properties, images, MaxImageCount) {}

        ImageBase(VkImage* vk_images, VkImageView* imageViews, size_t imageCount) :
                Image(vk_images, imageViews, imageCount, images) {}

        void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, int frame) override
        {
            frame = std::min(frame, maxImages-1);
            Image::WriteDescriptor(device, descriptorSet, binding, element, type, images[frame]);
        }

        void changeLayout(ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(images[index], newLayout, 1);
        }

        void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(commandBuffer, images[index], newLayout, 1);
        }

        ImageData images[MaxImageCount];

        static constexpr int maxImages = MaxImageCount;
    };

    template <int MaxImageCount>
    class StagedImageBase : BindResource, Image
    {
    public:
        StagedImageBase(uint32_t width, uint32_t height, uint32_t depth,
                Format format,
                VkImageUsageFlags usage,
                VkMemoryPropertyFlags properties) :
                Image(width, height, depth, format, usage, properties, images, staging, MaxImageCount)
        {}

        void changeLayout(ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(images[index], (VkImageLayout)newLayout, 1);
        }

        void changeLayout(VkCommandBuffer commandBuffer, ImageLayout newLayout, uint32_t index)
        {
            Image::changeLayout(commandBuffer, images[index], (VkImageLayout)newLayout, 1);
        }

        void writeDescriptor(VkDevice device, VkDescriptorSet descriptorSet, uint32_t binding, uint32_t element, VkDescriptorType type, int frame) override
        {
            frame = std::min(frame, maxImages-1);
            Image::WriteDescriptor(device, descriptorSet, binding, element, type, images[frame]);
        }

        void write(char* data, uint32_t index) 
        { 
            Image::Write(images[index], staging[index], data); 
        }

        void write(VkCommandBuffer commandBuffer, char* data, uint32_t index)
        {
            Image::Write(commandBuffer, images[index], staging[index], data);
        }

        ImageData images[MaxImageCount];
        StagingData staging[MaxImageCount];

        static constexpr int maxImages = MaxImageCount;
    };

    using SwapImage = class ImageBase<2>;
    using SharedImage = class ImageBase<1>;
    using StagedSwapImage = class StagedImageBase<2>;
    using StagedSharedImage = class StagedImageBase<1>;
    using AttachmentImage = class ImageBase<3>;
    using StagedAttachmentImage = class StagedImageBase<3>;
}