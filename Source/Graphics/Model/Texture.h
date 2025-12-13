#pragma once
#include "Graphics/Buffers/Buffer.h"

namespace texture::utils {
	bool load_texture_from_file(const char* filePath, VkImage& outImage, VkDeviceMemory& outImageMemory);
}

class Texture {
public:
	Texture(const std::string& path);
	Texture(const Texture&) = delete;
	Texture& operator=(const Texture&) = delete;
	~Texture();

	std::string& GetPath() { return _Path; }
	VkImage GetImage() const { return _Image; }
	VkDeviceMemory GetMemory() const { return _Memory; }
	VkImageView GetImageView() const { return _ImageView; }
	VkSampler GetSampler() const { return _Sampler; }

	VkDescriptorImageInfo DescriptorInfo() const {
		VkDescriptorImageInfo info{};
		info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		info.imageView = _ImageView;
		info.sampler = _Sampler;
		return info;
	}
private:
	bool CreateImageView(VkImage image, VkFormat format);
	bool CreateSampler();
private:
	std::string _Path;
	VkImage _Image = VK_NULL_HANDLE;
	VkDeviceMemory _Memory = VK_NULL_HANDLE;
	VkImageView _ImageView = VK_NULL_HANDLE;
	VkSampler _Sampler = VK_NULL_HANDLE;
};