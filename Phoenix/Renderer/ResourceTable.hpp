#pragma once

#include <Renderer/Vulkan.hpp>

class RenderDevice;
class Buffer;
class Texture;
class ResourceTableLayout;

class ResourceTable
{
public:
	ResourceTable(RenderDevice* device, ResourceTableLayout* resourceTableLayout, VkDescriptorSet descriptorSet);
	~ResourceTable() = default;

	VkDescriptorSet GetDescriptorSet() const;

	void Use(VkCommandBuffer* commandBuffer, uint32_t index, uint32_t set, VkPipelineLayout layout,
	         VkPipelineBindPoint bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS) const;

	void Bind(uint32_t binding, Buffer* buffer) const;
	void Bind(uint32_t binding, Texture* texture, uint32_t arrayElement = 0) const;

private:
	RenderDevice*        m_device;
	ResourceTableLayout* m_resourceTableLayout;
	VkDescriptorSet      m_descriptorSet;
};
