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
	RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, bool useDepth = false);
	RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, bool useDepth = false);
	~RenderTarget();

	RenderPass* GetRenderPass() const;
	Texture*    GetImage() const;

	void ScreenResize(uint32_t width, uint32_t height);

	void StartRendering(VkCommandBuffer* commandBuffer, uint32_t index) const;

	void StartSampling(VkCommandBuffer* commandBuffer, uint32_t index, Pipeline* pipeline) const;

	void StopSampling(VkCommandBuffer* commandBuffer, uint32_t index) const;

private:
	void CreateRenderTarget(uint32_t width, uint32_t height);
	void DestroyRenderTarget();

	RenderDevice*                          m_device;
	std::unique_ptr<RenderPass>            m_renderpass;
	std::unique_ptr<Texture>               m_image;
	std::unique_ptr<Texture>               m_depthImage;
	std::unique_ptr<FramebufferPacket>     m_framebufferPacket;
	std::unique_ptr<FramebufferPacket>     m_depthFramebufferPacket;
	std::unique_ptr<FramebufferAttachment> m_framebufferAttachment;
	std::unique_ptr<ResourceTable>         m_samplerResourceTable;
	VkFormat                               m_format;
	bool                                   m_useDepth;
};
