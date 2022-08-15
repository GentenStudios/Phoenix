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
	VkImage               GetImage() { return mImage; }
	VkSampler             GetSampler() { return mSampler; }
	VkImageView           GetImageView() { return mImageView; }
	VkFormat              GetFormat() { return mFormat; }
	uint32_t              GetWidth() { return mWidth; }
	uint32_t              GetHeight() { return mHeight; }

private:
	void CreateImageFrom(const VkImageCreateInfo& imageCreateInfo);
	void TransferData(const VkImageCreateInfo& imageCreateInfo, char* data);
	void CreateSampler(const VkImageCreateInfo& imageCreateInfo);
	void CreateImageView(const VkImageCreateInfo& imageCreateInfo);

	void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	                    VkImageSubresourceRange subresourceRange);

	RenderDevice* mDevice;

	bool         mOwnMemory;
	MemoryHeap*  mMemoryHeap;
	VkDeviceSize mImageSize;
	VkDeviceSize mMemoryOffset;

	uint32_t mWidth;
	uint32_t mHeight;
	uint32_t mDepth;
	uint32_t mLayers;
	uint32_t mMips;

	VkImage     mImage;
	VkSampler   mSampler;
	VkImageView mImageView;
	VkFormat    mFormat;

	VkImageUsageFlags mImageUsageFlags;
};