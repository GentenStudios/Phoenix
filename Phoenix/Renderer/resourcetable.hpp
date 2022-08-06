#pragma once

#include <Renderer/vulkan.hpp>

class RenderDevice;
class Buffer;
class Texture;
class ResourceTableLayout;

class ResourceTable
{
public:
	ResourceTable( RenderDevice* device, ResourceTableLayout* resourceTableLayout, VkDescriptorSet descriptorSet );
	~ResourceTable( );

	VkDescriptorSet GetDescriptorSet( ) { return mDescriptorSet; }

	void Use( VkCommandBuffer* commandBuffer, uint32_t index, uint32_t set, VkPipelineLayout layout, VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS );

	void Bind( uint32_t binding, Buffer* buffer );

	void Bind( uint32_t binding, Texture* texture, uint32_t arrayElement = 0 );

private:
	RenderDevice* mDevice;
	ResourceTableLayout* mResourceTableLayout;
	VkDescriptorSet mDescriptorSet;
};