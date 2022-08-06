#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Texture.hpp>

ResourceTable::ResourceTable(RenderDevice* device, ResourceTableLayout* resourceTableLayout, VkDescriptorSet descriptorSet)
    : mDevice(device), mResourceTableLayout(resourceTableLayout), mDescriptorSet(descriptorSet)
{
}

ResourceTable::~ResourceTable() {}

void ResourceTable::Use(VkCommandBuffer* commandBuffer, uint32_t index, uint32_t set, VkPipelineLayout layout,
                        VkPipelineBindPoint bindPoint)
{
	vkCmdBindDescriptorSets(commandBuffer[index], bindPoint, layout, set, 1, &mDescriptorSet, 0, NULL);
}

void ResourceTable::Bind(uint32_t binding, Buffer* buffer)
{
	VkDescriptorBufferInfo descriptorBufferInfos[] = {buffer->GetDescriptorInfo()};

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet               = mDescriptorSet;
	descriptorWrite.dstBinding           = binding;
	descriptorWrite.dstArrayElement      = 0;
	descriptorWrite.descriptorType       = mResourceTableLayout->GetDescriptorType(binding);
	descriptorWrite.descriptorCount      = 1;
	descriptorWrite.pBufferInfo          = descriptorBufferInfos;
	descriptorWrite.pImageInfo           = VK_NULL_HANDLE;
	descriptorWrite.pTexelBufferView     = VK_NULL_HANDLE;
	descriptorWrite.pNext                = VK_NULL_HANDLE;

	vkUpdateDescriptorSets(mDevice->GetDevice(), 1, &descriptorWrite, 0, NULL);
}

void ResourceTable::Bind(uint32_t binding, Texture* texture, uint32_t arrayElement)
{
	if (!texture)
		return;
	VkDescriptorImageInfo descriptorImageInfo = texture->GetDescriptorImageInfo();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet               = mDescriptorSet;
	descriptorWrite.dstBinding           = binding;
	descriptorWrite.dstArrayElement      = arrayElement;
	descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount      = 1;
	descriptorWrite.pBufferInfo          = VK_NULL_HANDLE;
	descriptorWrite.pImageInfo           = VK_NULL_HANDLE;
	descriptorWrite.pTexelBufferView     = VK_NULL_HANDLE;
	descriptorWrite.pNext                = VK_NULL_HANDLE;

	static const int        offset = offsetof(VkWriteDescriptorSet, pImageInfo);
	VkDescriptorImageInfo** data   = reinterpret_cast<VkDescriptorImageInfo**>(reinterpret_cast<uint8_t*>(&descriptorWrite) + offset);
	*data                          = &descriptorImageInfo;

	vkUpdateDescriptorSets(mDevice->GetDevice(), 1, &descriptorWrite, 0, NULL);
}
