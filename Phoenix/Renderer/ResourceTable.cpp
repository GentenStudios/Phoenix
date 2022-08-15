#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Texture.hpp>

ResourceTable::ResourceTable(RenderDevice* device, ResourceTableLayout* resourceTableLayout, VkDescriptorSet descriptorSet)
    : m_device(device), m_resourceTableLayout(resourceTableLayout), m_descriptorSet(descriptorSet)
{
}

VkDescriptorSet ResourceTable::GetDescriptorSet() const { return m_descriptorSet; }

void ResourceTable::Use(VkCommandBuffer* commandBuffer, uint32_t index, uint32_t set, VkPipelineLayout layout,
                        VkPipelineBindPoint bindPoint) const
{
	vkCmdBindDescriptorSets(commandBuffer[index], bindPoint, layout, set, 1, &m_descriptorSet, 0, nullptr);
}

void ResourceTable::Bind(uint32_t binding, Buffer* buffer) const
{
	VkDescriptorBufferInfo descriptorBufferInfos[] = {buffer->GetDescriptorInfo()};

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet               = m_descriptorSet;
	descriptorWrite.dstBinding           = binding;
	descriptorWrite.dstArrayElement      = 0;
	descriptorWrite.descriptorType       = m_resourceTableLayout->GetDescriptorType(binding);
	descriptorWrite.descriptorCount      = 1;
	descriptorWrite.pBufferInfo          = descriptorBufferInfos;
	descriptorWrite.pImageInfo           = VK_NULL_HANDLE;
	descriptorWrite.pTexelBufferView     = VK_NULL_HANDLE;
	descriptorWrite.pNext                = VK_NULL_HANDLE;

	vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

void ResourceTable::Bind(uint32_t binding, Texture* texture, uint32_t arrayElement) const
{
	if (!texture)
		return;

	VkDescriptorImageInfo descriptorImageInfo = texture->GetDescriptorImageInfo();

	VkWriteDescriptorSet descriptorWrite = {};
	descriptorWrite.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet               = m_descriptorSet;
	descriptorWrite.dstBinding           = binding;
	descriptorWrite.dstArrayElement      = arrayElement;
	descriptorWrite.descriptorType       = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrite.descriptorCount      = 1;
	descriptorWrite.pBufferInfo          = VK_NULL_HANDLE;
	descriptorWrite.pImageInfo           = VK_NULL_HANDLE;
	descriptorWrite.pTexelBufferView     = VK_NULL_HANDLE;
	descriptorWrite.pNext                = VK_NULL_HANDLE;

	static const int offset = offsetof(VkWriteDescriptorSet, pImageInfo);
	auto**           data   = reinterpret_cast<VkDescriptorImageInfo**>(reinterpret_cast<uint8_t*>(&descriptorWrite) + offset);
	*data                   = &descriptorImageInfo;

	vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}
