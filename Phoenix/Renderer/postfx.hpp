#pragma once

#include <Renderer/vulkan.hpp>

#include <memory>
#include <vector>

class PipelineLayout;
class Pipeline;
class RenderDevice;
class RenderTarget;

class PostFX
{
public:
	PostFX( RenderDevice* device );
	virtual ~PostFX( ) { };
	virtual void Use( VkCommandBuffer* commandBuffer, uint32_t index, RenderTarget* src, RenderTarget* dst ) = 0;
	PipelineLayout* GetPipelineLayout(){return mPipelineLayout.get( ); }
	Pipeline* GetPipeline( ){ return mPipeline.get( ); }
protected:
	RenderDevice* mDevice;
	std::unique_ptr<PipelineLayout> mPipelineLayout;
	std::unique_ptr<Pipeline> mPipeline;
	VkShaderModule mVertexShader;
	VkShaderModule mFragmentShader;
};