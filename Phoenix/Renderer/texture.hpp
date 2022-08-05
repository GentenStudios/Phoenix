#pragma once

#include <vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class Buffer;
class MemoryHeap;

class Texture
{
public:
	Texture( RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags, char* data = nullptr );
	Texture( RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags, char* data = nullptr );
	~Texture( );

	void TransitionImageLayout( VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout );

	VkDescriptorImageInfo GetDescriptorImageInfo( );
	VkImage GetImage( ) { return mImage; }
	VkSampler GetSampler( ) { return mSampler; }
	VkImageView GetImageView( ) { return mImageView; }
	VkFormat GetFormat( ) { return mFormat; }
	uint32_t GetWidth( ) { return mWidth; }
	uint32_t GetHeight( ) { return mHeight; }
private:
	void CreateImage( VkImageUsageFlags imageUsageFlags, char* data );
	void SetImageLayout( VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange );

	RenderDevice* mDevice;
	MemoryHeap* mMemoryHeap;
	bool mOwnMemory;
	uint32_t mWidth;
	uint32_t mHeight;
	VkImage mImage;
	VkSampler mSampler;
	VkImageView mImageView;
	VkFormat mFormat;


	VkImageUsageFlags mImageUsageFlags;
};