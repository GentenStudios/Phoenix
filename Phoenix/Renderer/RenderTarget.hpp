#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class RenderPass;
class Texture;
class FramebufferAttachment;
class FramebufferPacket;
class ResourceTableLayout;
class ResourceTable;
class Pipeline;

class RenderTarget
{
public:
	RenderTarget( RenderDevice* device, uint32_t width, uint32_t height, bool useDepth = false );
	RenderTarget( RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, bool useDepth = false );
	~RenderTarget( );

	RenderPass* GetRenderPass( ) { return mRenderpass.get( ); }
	Texture* GetImage( ) { return mImage.get( ); }

	void ScreenResize( uint32_t width, uint32_t height );

	void StartRendering( VkCommandBuffer* commandBuffer, uint32_t index );

	void StartSampling( VkCommandBuffer* commandBuffer, uint32_t index, Pipeline* pipeline );

	void StopSampling( VkCommandBuffer* commandBuffer, uint32_t index );

private:
	void CreateRenderTarget( uint32_t width, uint32_t height );
	void DestroyRenderTarget( );

	RenderDevice* mDevice;
	std::unique_ptr<RenderPass> mRenderpass;
	std::unique_ptr<Texture> mImage;
	std::unique_ptr<Texture> mDepthImage;
	std::unique_ptr<FramebufferPacket> mFramebufferPacket;
	std::unique_ptr<FramebufferPacket> mDepthFramebufferPacket;
	std::unique_ptr<FramebufferAttachment> mFramebufferAttachment;
	std::unique_ptr<ResourceTable> mSamplerResourceTable;
	VkFormat mFormat;
	bool mUseDepth;
};