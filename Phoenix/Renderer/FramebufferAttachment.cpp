
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/FramebufferAttachment.hpp>
#include <Renderer/Texture.hpp>

FramebufferPacket::FramebufferPacket(Texture* textures, uint32_t textureCount, VkFormat format, VkImageLayout layout,
                                     EFramebufferImageType imageType)
    : m_imageViewCount(textureCount), m_format(format), m_layout(layout), m_imageType(imageType)
{
	m_imageViews = std::unique_ptr<VkImageView>(new VkImageView[textureCount]);
	for (uint32_t i = 0; i < textureCount; i++)
	{
		VkImageView im       = textures[i].GetImageView();
		m_imageViews.get()[i] = im;
	}
}

FramebufferPacket::FramebufferPacket(VkImageView* imageViews, uint32_t textureCount, VkFormat format, VkImageLayout layout,
                                     EFramebufferImageType imageType)
    : m_imageViewCount(textureCount), m_format(format), m_layout(layout), m_imageType(imageType)
{
	m_imageViews = std::unique_ptr<VkImageView>(new VkImageView[textureCount]);
	for (uint32_t i = 0; i < textureCount; i++)
	{
		VkImageView im       = imageViews[i];
		m_imageViews.get()[i] = im;
	}
}

FramebufferPacket::~FramebufferPacket() {}

VkImageView* FramebufferPacket::GetImageViews() const { return m_imageViews.get(); }

uint32_t FramebufferPacket::GetImageViewCount() const { return m_imageViewCount; }

VkFormat FramebufferPacket::GetFormat() const { return m_format; }

VkImageLayout FramebufferPacket::GetImageLayout() const { return m_layout; }

EFramebufferImageType FramebufferPacket::GetImageType() const { return m_imageType; }

FramebufferAttachment::FramebufferAttachment(RenderDevice* device, std::vector<FramebufferPacket*> framebufferPackets)
    : m_device(device), m_framebufferAttachmentCount(static_cast<uint32_t>(framebufferPackets.size())),
      m_framebufferPackets(framebufferPackets)
{
	m_framebufferCount = m_device->GetSwapchainImageCount();

	m_framebufferAttachments = new VkImageView*[m_framebufferCount];

	for (uint32_t i = 0; i < m_framebufferCount; i++)
	{
		m_framebufferAttachments[i] = new VkImageView[m_framebufferAttachmentCount];
		for (uint32_t j = 0; j < m_framebufferAttachmentCount; j++)
		{
			m_framebufferAttachments[i][j] = framebufferPackets[j]->GetImageViews()[i % framebufferPackets[j]->GetImageViewCount()];
		}
	}
}

FramebufferAttachment::~FramebufferAttachment()
{
	for (uint32_t i = 0; i < m_framebufferCount; i++)
	{
		delete[] m_framebufferAttachments[i];
	}
	delete[] m_framebufferAttachments;
}

VkImageView* FramebufferAttachment::GetFramebufferAttachments(unsigned swapchainImageIndex) const
{
	return m_framebufferAttachments[swapchainImageIndex];
}

uint32_t FramebufferAttachment::GetFramebufferAttachmentCount() const { return m_framebufferAttachmentCount; }
