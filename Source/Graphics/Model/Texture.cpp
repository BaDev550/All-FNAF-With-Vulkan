#include "gpch.h"
#include "Texture.h"
#include "Engine/Application.h"

#include "stb_image.h"

#include <stdexcept>
#include <cstring>

namespace texture::utils {
    bool load_texture_from_file(const char* filePath, VkImage& outImage, VkDeviceMemory& outImageMemory, uint32_t& outWidth, uint32_t& outHeight);
    VkImageView createImageView(VkImage image, VkFormat format);
}

Texture::Texture(const std::string& path) 
    : _Path(path)
{
    uint32_t width = 0, height = 0;
    if (!texture::utils::load_texture_from_file(path.c_str(), _Image, _Memory, width, height)) {
        throw std::runtime_error("Failed to load texture: " + path);
    }

    _ImageView = texture::utils::createImageView(_Image, VK_FORMAT_R8G8B8A8_SRGB);
    if (_ImageView == VK_NULL_HANDLE) {
        throw std::runtime_error("Failed to create image view for texture: " + path);
    }

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = 16.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    assert(vkCreateSampler(Application::Get()->GetDevice().LogicalDevice(), &samplerInfo, nullptr, &_Sampler) == VK_SUCCESS && "Failed to create texture sampler");
}

Texture::~Texture() {
    if (_Sampler != VK_NULL_HANDLE) {
        vkDestroySampler(Application::Get()->GetDevice().LogicalDevice(), _Sampler, nullptr);
        _Sampler = VK_NULL_HANDLE;
    }
    if (_ImageView != VK_NULL_HANDLE) {
        vkDestroyImageView(Application::Get()->GetDevice().LogicalDevice(), _ImageView, nullptr);
        _ImageView = VK_NULL_HANDLE;
    }
    if (_Image != VK_NULL_HANDLE) {
        vkDestroyImage(Application::Get()->GetDevice().LogicalDevice(), _Image, nullptr);
        _Image = VK_NULL_HANDLE;
    }
    if (_Memory != VK_NULL_HANDLE) {
        vkFreeMemory(Application::Get()->GetDevice().LogicalDevice(), _Memory, nullptr);
        _Memory = VK_NULL_HANDLE;
    }
}

namespace texture::utils {

    static void TransitionImageLayout(Device& device, VkCommandBuffer cmd, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange = subresourceRange;

        VkPipelineStageFlags srcStage = 0;
        VkPipelineStageFlags dstStage = 0;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = 0;
            srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            dstStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        }

        vkCmdPipelineBarrier(
            cmd,
            srcStage, dstStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    }

    VkImageView createImageViewInternal(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(Application::Get()->GetDevice().LogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            return VK_NULL_HANDLE;
        }
        return imageView;
    }

    VkImageView createImageView(VkImage image, VkFormat format) {
        return createImageViewInternal(image, format, VK_IMAGE_ASPECT_COLOR_BIT);
    }

    bool load_texture_from_file(const char* filePath, VkImage& outImage, VkDeviceMemory& outImageMemory, uint32_t& outWidth, uint32_t& outHeight) {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(filePath, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if (!pixels) {
            std::cerr << "Failed to load texture from: " << filePath << std::endl;
            return false;
        }

        outWidth = static_cast<uint32_t>(texWidth);
        outHeight = static_cast<uint32_t>(texHeight);
        VkDeviceSize imageSize = static_cast<VkDeviceSize>(outWidth) * outHeight * 4ULL;

        Buffer stagingBuffer{
            imageSize,
            1,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        };
        stagingBuffer.Map();
        stagingBuffer.WriteToBuffer(pixels, imageSize);
        stagingBuffer.Unmap();

        stbi_image_free(pixels);

        VkExtent3D imageExtent{};
        imageExtent.width = outWidth;
        imageExtent.height = outHeight;
        imageExtent.depth = 1;

        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent = imageExtent;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        Application::Get()->GetDevice().CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, outImage, outImageMemory);

        VkCommandBuffer cmd = Application::Get()->GetDevice().BeginSingleTimeCommands();
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel = 0;
        subresourceRange.levelCount = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount = 1;

        TransitionImageLayout(Application::Get()->GetDevice(), cmd, outImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = { 0,0,0 };
        region.imageExtent = { outWidth, outHeight, 1 };

        vkCmdCopyBufferToImage(cmd, stagingBuffer.GetBuffer(), outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        TransitionImageLayout(Application::Get()->GetDevice(), cmd, outImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

        Application::Get()->GetDevice().EndSingleTimeCommands(cmd);
        return true;
    }
}