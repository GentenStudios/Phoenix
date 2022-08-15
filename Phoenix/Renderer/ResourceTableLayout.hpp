#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class ResourceTable;

class ResourceTableLayout
{
public:
	ResourceTableLayout(RenderDevice* device, VkDescriptorSetLayoutBinding* descriptorSetLayoutBinding,
	                    uint32_t descriptorSetLayoutBindingCount, uint32_t maxSets);
	~ResourceTableLayout();

	ResourceTable* CreateTable();

	VkDescriptorPool      GetDescriptorPool() const;
	VkDescriptorSetLayout GetDescriptorSetLayout() const;

	VkDescriptorType GetDescriptorType(uint32_t index) const;

private:
	RenderDevice*                                 m_device;
	VkDescriptorPool                              m_descriptorPool      = VK_NULL_HANDLE;
	VkDescriptorSetLayout                         m_descriptorSetLayout = VK_NULL_HANDLE;
	std::unique_ptr<VkDescriptorSetLayoutBinding> m_descriptorSetLayoutBindings;
	uint32_t                                      m_descriptorSetLayoutBindingCount;
};
