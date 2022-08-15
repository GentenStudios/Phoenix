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

#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>

#include <cassert>
#include <cstring>

ResourceTableLayout::ResourceTableLayout(RenderDevice* device, VkDescriptorSetLayoutBinding* descriptorSetLayoutBinding,
                                         uint32_t descriptorSetLayoutBindingCount, uint32_t maxSets)
    : m_device(device)
{
	m_descriptorSetLayoutBindings =
	    std::unique_ptr<VkDescriptorSetLayoutBinding>(new VkDescriptorSetLayoutBinding[descriptorSetLayoutBindingCount]);

	std::memcpy(m_descriptorSetLayoutBindings.get(), descriptorSetLayoutBinding,
	            sizeof(VkDescriptorSetLayoutBinding) * descriptorSetLayoutBindingCount);

	m_descriptorSetLayoutBindingCount = descriptorSetLayoutBindingCount;

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount                    = descriptorSetLayoutBindingCount;
	descriptorSetLayoutCreateInfo.pBindings                       = descriptorSetLayoutBinding;

	m_device->Validate(vkCreateDescriptorSetLayout(m_device->GetDevice(), &descriptorSetLayoutCreateInfo, nullptr, &m_descriptorSetLayout));

	auto descriptorPoolSizes = std::make_unique<VkDescriptorPoolSize[]>(descriptorSetLayoutBindingCount);

	for (uint32_t i = 0; i < descriptorSetLayoutBindingCount; ++i)
	{
		descriptorPoolSizes[i].type            = descriptorSetLayoutBinding[i].descriptorType;
		descriptorPoolSizes[i].descriptorCount = maxSets;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType                      = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount              = descriptorSetLayoutBindingCount;
	descriptorPoolCreateInfo.pPoolSizes                 = descriptorPoolSizes.get();
	descriptorPoolCreateInfo.maxSets                    = maxSets;

	m_device->Validate(vkCreateDescriptorPool(m_device->GetDevice(), &descriptorPoolCreateInfo, nullptr, &m_descriptorPool));
}

ResourceTableLayout::~ResourceTableLayout()
{
	vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_descriptorSetLayout, nullptr);
	vkDestroyDescriptorPool(m_device->GetDevice(), m_descriptorPool, nullptr);
}

ResourceTable* ResourceTableLayout::CreateTable()
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool              = m_descriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount          = 1;
	descriptorSetAllocateInfo.pSetLayouts                 = &m_descriptorSetLayout;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	m_device->Validate(vkAllocateDescriptorSets(m_device->GetDevice(), &descriptorSetAllocateInfo, &descriptorSet));

	return new ResourceTable(m_device, this, descriptorSet);
}

VkDescriptorPool ResourceTableLayout::GetDescriptorPool() const { return m_descriptorPool; }

VkDescriptorSetLayout ResourceTableLayout::GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

VkDescriptorType ResourceTableLayout::GetDescriptorType(uint32_t index) const
{
	assert(index < m_descriptorSetLayoutBindingCount);
	return m_descriptorSetLayoutBindings.get()[index].descriptorType;
}

