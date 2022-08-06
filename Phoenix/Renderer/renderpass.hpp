#pragma once

#include <Renderer/vulkan.hpp>

#include <memory>

class RenderDevice;
class FramebufferAttachment;
class FramebufferPacket;

class RenderPass
{
public:
	RenderPass( RenderDevice* device, uint32_t width, uint32_t height, FramebufferAttachment* framebufferAttachment );
	~RenderPass( );

	void Use( VkCommandBuffer* commandBuffer, uint32_t index );

	VkRenderPass GetRenderPass( ) { return mRenderPass; }
	VkFramebuffer* GetFrameBuffers( ) { return mFramebuffers.get(); }

	uint32_t GetPipelineColorBlendAttachmentStateCount( ) { return mPipelineColorBlendAttachmentStateCount; }
	VkPipelineColorBlendAttachmentState* GetPipelineColorBlendAttachmentStates( ) { return mPipelineColorBlendAttachmentStates; }

	void Rebuild( FramebufferAttachment* framebufferAttachment, uint32_t width, uint32_t height );

private:

	void CreateFrameBuffer( uint32_t width, uint32_t height);
	void DestroyFrameBuffer( );

	RenderDevice* mDevice;
	uint32_t mWidth;
	uint32_t mHeight;
	FramebufferAttachment* mFramebufferAttachment;
	VkRenderPass mRenderPass = VK_NULL_HANDLE;
	uint32_t mPipelineColorBlendAttachmentStateCount;
	VkPipelineColorBlendAttachmentState* mPipelineColorBlendAttachmentStates;
	std::unique_ptr<VkFramebuffer> mFramebuffers = nullptr;
};