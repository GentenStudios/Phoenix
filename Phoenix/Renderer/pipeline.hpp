#pragma once

#include <Renderer/vulkan.hpp>

class RenderDevice;
class RenderPass;
class PipelineLayout;

enum class PipelineType
{
	Graphics,
	Compute
};

class Pipeline
{
public:
	Pipeline( RenderDevice* device, PipelineType type, RenderPass* renderPass, PipelineLayout* pipelineLayout,
		VkPipelineShaderStageCreateInfo* pipelineShaderStageInfos, uint32_t pipelineShaderStageInfoCount, 
		VkVertexInputBindingDescription* vertexInputBindingDescriptions, uint32_t vertexInputBindingDescriptionCount,
		VkVertexInputAttributeDescription* vertexInputAttributeDescriptions, uint32_t vertexInputAttributeDescriptionCount,
		VkPrimitiveTopology topology);
	~Pipeline( );

	void Use( VkCommandBuffer* commandBuffer, uint32_t index );

	VkPipelineLayout GetPipelineLayout( );

private:
	RenderDevice* mDevice;
	PipelineLayout* mPipelineLayout;
	VkPipeline mPipeline;
	PipelineType mType;
};