#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class Buffer;
class MemoryHeap;

class Texture
{
public:
	Texture(RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format,
	        VkImageUsageFlags imageUsageFlags, char* data = nullptr);

	Texture(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags,
	        char* data = nullptr);

	Texture(RenderDevice* device, MemoryHeap* memoryHeap, const VkImageCreateInfo& imageCreateInfo, char* data = nullptr);
	~Texture();

	void CopyBufferRegionsToImage(Buffer* buffer, VkBufferImageCopy* copies, uint32_t count);
	void TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkDescriptorImageInfo GetDescriptorImageInfo();
	VkImage               GetImage() const;
	VkSampler             GetSampler() const;
	VkImageView           GetImageView() const;
	VkFormat              GetFormat() const;
	uint32_t              GetWidth() const;
	uint32_t              GetHeight() const;

private:
	void CreateImageFrom(const VkImageCreateInfo& imageCreateInfo);
	void TransferData(const VkImageCreateInfo& imageCreateInfo, char* data);
	void CreateSampler(const VkImageCreateInfo& imageCreateInfo);
	void CreateImageView(const VkImageCreateInfo& imageCreateInfo);

	void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	                    VkImageSubresourceRange subresourceRange);

	RenderDevice* m_device;

	bool         m_ownMemory;
	MemoryHeap*  m_memoryHeap;
	VkDeviceSize m_imageSize;
	VkDeviceSize m_memoryOffset;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	uint32_t m_layers;
	uint32_t m_mips;

	VkImage     m_image;
	VkSampler   m_sampler;
	VkImageView m_imageView;
	VkFormat    m_format;

	VkImageUsageFlags m_imageUsageFlags;
};
