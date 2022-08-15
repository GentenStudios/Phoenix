#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>
#include <vector>

class PipelineLayout;
class Pipeline;
class RenderDevice;
class RenderTarget;

class PostFX
{
public:
	explicit PostFX(RenderDevice* device);
	virtual ~PostFX() = default;

	virtual void    Use(VkCommandBuffer* commandBuffer, uint32_t index, RenderTarget* src, RenderTarget* dst) = 0;

	PipelineLayout* GetPipelineLayout() const { return m_pipelineLayout.get(); }
	Pipeline*       GetPipeline() const { return m_pipeline.get(); }

protected:
	RenderDevice*                   m_device;

	std::unique_ptr<PipelineLayout> m_pipelineLayout;
	std::unique_ptr<Pipeline>       m_pipeline;

	VkShaderModule m_vertexShader   = VK_NULL_HANDLE;
	VkShaderModule m_fragmentShader = VK_NULL_HANDLE;
};
