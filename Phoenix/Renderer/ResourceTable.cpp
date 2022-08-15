// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

