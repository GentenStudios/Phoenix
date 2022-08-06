#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/FramebufferAttachment.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/Renderpass.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Texture.hpp>

RenderTarget::RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, bool useDepth) : mDevice(device), mUseDepth(useDepth)
{
	mFormat = mDevice->GetSurfaceFormat();
	CreateRenderTarget(width, height);
	mRenderpass = std::unique_ptr<RenderPass>(new RenderPass(mDevice, width, height, mFramebufferAttachment.get()));
}

RenderTarget::RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, bool useDepth)
    : mDevice(device), mUseDepth(useDepth), mFormat(format)
{
	CreateRenderTarget(width, height);
	mRenderpass = std::unique_ptr<RenderPass>(new RenderPass(mDevice, width, height, mFramebufferAttachment.get()));
}

RenderTarget::~RenderTarget()
{
	DestroyRenderTarget();
	mRenderpass.reset();
	mSamplerResourceTable.reset();
	mDepthImage.reset();
	mDepthFramebufferPacket.reset();
}

void RenderTarget::ScreenResize(uint32_t width, uint32_t height)
{
	DestroyRenderTarget();
	CreateRenderTarget(width, height);
	mRenderpass->Rebuild(mFramebufferAttachment.get(), width, height);
}

void RenderTarget::StartRendering(VkCommandBuffer* commandBuffer, uint32_t index)
{
	VkViewport viewport = {};
	viewport.x          = 0;
	viewport.y          = 0;
	viewport.width      = (float) mImage->GetWidth();
	viewport.height     = (float) mImage->GetHeight();
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissor {};
	scissor.extent.width  = mImage->GetWidth();
	scissor.extent.height = mImage->GetHeight();
	scissor.offset.x      = 0;
	scissor.offset.y      = 0;

	mRenderpass->Use(commandBuffer, index);

	vkCmdSetViewport(commandBuffer[index], 0, 1, &viewport);

	vkCmdSetScissor(commandBuffer[index], 0, 1, &scissor);
}

void RenderTarget::StartSampling(VkCommandBuffer* commandBuffer, uint32_t index, Pipeline* pipeline)
{
	mImage->TransitionImageLayout(commandBuffer[index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	mSamplerResourceTable->Use(commandBuffer, index, 0, pipeline->GetPipelineLayout());
}

void RenderTarget::StopSampling(VkCommandBuffer* commandBuffer, uint32_t index)
{
	mImage->TransitionImageLayout(commandBuffer[index], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void RenderTarget::CreateRenderTarget(uint32_t width, uint32_t height)
{
	mImage = std::unique_ptr<Texture>(new Texture(mDevice, width, height, mFormat,
	                                              VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
	                                                  VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	                                              nullptr));

	mFramebufferPacket = std::unique_ptr<FramebufferPacket>(
	    new FramebufferPacket(mImage.get(), 1, mFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, EFramebufferImageType::Color));

	if (mUseDepth)
	{
		mDepthImage = std::unique_ptr<Texture>(
		    new Texture(mDevice, width, height, mDevice->GetDepthFormat(),
		                VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, nullptr));
		mDepthFramebufferPacket = std::unique_ptr<FramebufferPacket>(new FramebufferPacket(
		    mDepthImage.get(), 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, EFramebufferImageType::Depth));

		mFramebufferAttachment = std::unique_ptr<FramebufferAttachment>(
		    new FramebufferAttachment(mDevice, {mFramebufferPacket.get(), mDepthFramebufferPacket.get()}));
	}
	else
	{
		mFramebufferAttachment = std::unique_ptr<FramebufferAttachment>(new FramebufferAttachment(mDevice, {mFramebufferPacket.get()}));
	}

	if (mSamplerResourceTable == nullptr)
	{
		mSamplerResourceTable = std::unique_ptr<ResourceTable>(mDevice->GetPostProcessSampler()->CreateTable());
	}
	mSamplerResourceTable->Bind(0, mImage.get());
}

void RenderTarget::DestroyRenderTarget()
{
	mImage.reset();
	mFramebufferAttachment.reset();
	mFramebufferPacket.reset();
}
