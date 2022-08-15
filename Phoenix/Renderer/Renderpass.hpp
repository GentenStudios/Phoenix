#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class FramebufferAttachment;
class FramebufferPacket;

class RenderPass
{
public:
	RenderPass(RenderDevice* device, uint32_t width, uint32_t height, FramebufferAttachment* framebufferAttachment);
	~RenderPass();

	void Use(VkCommandBuffer* commandBuffer, uint32_t index) const;

	VkRenderPass   GetRenderPass() const;
	VkFramebuffer* GetFrameBuffers() const;

	uint32_t                             GetPipelineColorBlendAttachmentStateCount() const;
	VkPipelineColorBlendAttachmentState* GetPipelineColorBlendAttachmentStates() const;

	void Rebuild(FramebufferAttachment* framebufferAttachment, uint32_t width, uint32_t height);

private:
	void CreateFrameBuffer(uint32_t width, uint32_t height);
	void DestroyFrameBuffer();

private:
	RenderDevice*                                          m_device;
	uint32_t                                               m_width;
	uint32_t                                               m_height;
	FramebufferAttachment*                                 m_framebufferAttachment;
	VkRenderPass                                           m_renderpass = VK_NULL_HANDLE;
	uint32_t                                               m_pipelineColorBlendAttachmentStateCount;
	std::unique_ptr<VkPipelineColorBlendAttachmentState[]> m_pipelineColorBlendAttachmentStates;
	std::unique_ptr<VkFramebuffer[]>                       m_framebuffers;
};
