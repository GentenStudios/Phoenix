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

	VkImageView*          GetImageViews() const;
	uint32_t              GetImageViewCount() const;
	VkFormat              GetFormat() const;
	VkImageLayout         GetImageLayout() const;
	EFramebufferImageType GetImageType() const;

private:
	std::unique_ptr<VkImageView> m_imageViews;
	uint32_t                     m_imageViewCount;
	VkFormat                     m_format;
	VkImageLayout                m_layout;
	EFramebufferImageType        m_imageType;
};

class FramebufferAttachment
{
public:
	FramebufferAttachment(RenderDevice* device, std::vector<FramebufferPacket*> framebufferPackets);
	~FramebufferAttachment();

	VkImageView* GetFramebufferAttachments(unsigned int swapchainImageIndex) const;
	uint32_t     GetFramebufferAttachmentCount() const;

	std::vector<FramebufferPacket*> GetFramebufferPackets() { return m_framebufferPackets; }

private:
	RenderDevice* m_device;
	uint32_t      m_framebufferAttachmentCount;
	uint32_t      m_framebufferCount;

	VkImageView**                   m_framebufferAttachments;
	std::vector<FramebufferPacket*> m_framebufferPackets;
};
