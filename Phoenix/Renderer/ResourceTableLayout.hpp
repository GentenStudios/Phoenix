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

	VkDescriptorPool      GetDescriptorPool() { return mDescriptorPool; }
	VkDescriptorSetLayout GetDescriptorSetLayout() { return mDescriptorSetLayout; }

	VkDescriptorType GetDescriptorType(uint32_t index);

private:
	RenderDevice*                                 mDevice;
	VkDescriptorPool                              mDescriptorPool      = VK_NULL_HANDLE;
	VkDescriptorSetLayout                         mDescriptorSetLayout = VK_NULL_HANDLE;
	std::unique_ptr<VkDescriptorSetLayoutBinding> mDescriptorSetLayoutBindings;
	uint32_t                                      mDescriptorSetLayoutBindingCount;
};