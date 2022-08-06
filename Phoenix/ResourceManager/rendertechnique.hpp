#pragma once

#include <Renderer/Vulkan.hpp>

#define MAX_SHADER_MODULES 100

class RenderDevice;
class Pipeline;
class PipelineLayout;
class ResourceManager;
class RenderPass;

class RenderTechnique
{
public:
	RenderTechnique( RenderDevice* device, ResourceManager* resourceManager, RenderPass* renderPass, const char* name, const char* path );
	~RenderTechnique( );

	PipelineLayout* GetPipelineLayout( ) { return mPipelineLayout; }

	Pipeline* GetPipeline( ) { return mPipeline; }
private:
	VkVertexInputRate GetVertexInputFromAttribute( const char* text );
	VkFormat GetFormatFromAttribute( const char* text );
	VkShaderStageFlagBits GetStageFromAttribute( const char* text );

	RenderDevice* mDevice;
	ResourceManager* mResourceManager;
	const char* mName;
	const char* mPath;
	VkShaderModule mShaderModule[MAX_SHADER_MODULES];
	uint32_t stageCount = 0;
	PipelineLayout* mPipelineLayout;
	Pipeline* mPipeline;
};