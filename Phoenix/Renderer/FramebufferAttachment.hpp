#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>
#include <vector>

class RenderDevice;
class DeviceMemory;
class Texture;

// struct SFrameBufferAttachmentInstance
//{
//	VkImage image;
//	VkDeviceMemory memory;
//	VkImageView view;
//	VkFormat format;
//	VkSampler sampler;
// };

enum class EFramebufferImageType
{
	Color,
	Depth
};

class FramebufferPacket
{
public:
	FramebufferPacket(Texture* textures, uint32_t textureCount, VkFormat format, VkImageLayout layout, EFramebufferImageType imageType);
	FramebufferPacket(VkImageView* imageViews, uint32_t textureCount, VkFormat format, VkImageLayout layout,
	                  EFramebufferImageType imageType);
	~FramebufferPacket();

	VkImageView*          GetImageViews() { return mImageViews.get(); }
	uint32_t              GetImageViewCount() { return mImageViewCount; }
	VkFormat              GetFormat() { return mFormat; }
	VkImageLayout         GetImageLayout() { return mLayout; }
	EFramebufferImageType GetImageType() { return mImageType; }

private:
	std::unique_ptr<VkImageView> mImageViews;
	uint32_t                     mImageViewCount;
	VkFormat                     mFormat;
	VkImageLayout                mLayout;
	EFramebufferImageType        mImageType;
};

class FramebufferAttachment
{
public:
	FramebufferAttachment(RenderDevice* device, std::vector<FramebufferPacket*> framebufferPackets);
	~FramebufferAttachment();

	VkImageView* GetFramebufferAttachments(unsigned int swapchainImageIndex) { return mFramebufferAttachments[swapchainImageIndex]; }
	uint32_t     GetFramebufferAttachmentCount() { return mFramebufferAttachmentCount; }

	std::vector<FramebufferPacket*> GetFramebufferPackets() { return mFramebufferPackets; }

private:
	RenderDevice* mDevice;
	uint32_t      mFramebufferAttachmentCount;
	uint32_t      mFramebufferCount;

	VkImageView**                   mFramebufferAttachments;
	std::vector<FramebufferPacket*> mFramebufferPackets;
};