#pragma once

#include <Renderer/Vulkan.hpp>

class RenderDevice;

class PipelineLayout
{
public:
	PipelineLayout( RenderDevice* device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount );
	~PipelineLayout( );

	VkPipelineLayout GetPipelineLayout( ) { return mPipelineLayout; }
private:
	RenderDevice* mDevice;
	VkPipelineLayout mPipelineLayout;
};