#pragma once

#include <Renderer/Vulkan.hpp>

class RenderDevice;

class PipelineLayout
{
public:
	PipelineLayout(RenderDevice* device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount);
	~PipelineLayout();

	VkPipelineLayout GetPipelineLayout() const;

private:
	RenderDevice*    m_device;
	VkPipelineLayout m_pipelineLayout;
};
