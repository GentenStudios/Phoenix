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
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>

#include <cassert>
#include <cstring>

Buffer::Buffer(RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
    : m_device(device), m_memoryHeap(memoryHeap), m_bufferSize(static_cast<uint32_t>(size))
{
	m_deviceMemory = m_memoryHeap->GetMemory();

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = usage;
	bufferCreateInfo.sharingMode        = sharingMode;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &m_buffer));

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &bufferMemoryRequirements);

	m_allocationSize = static_cast<uint32_t>(bufferMemoryRequirements.size);
	m_memoryOffset   = m_memoryHeap->Allocate(m_allocationSize, static_cast<uint32_t>(bufferMemoryRequirements.alignment));

	m_device->Validate(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_memoryHeap->GetMemory()->GetMemory(), m_memoryOffset));
}

Buffer::Buffer(RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
    : m_device(device), m_bufferSize(static_cast<uint32_t>(size))
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferCreateInfo.sharingMode        = sharingMode;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &m_buffer));

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &bufferMemoryRequirements);

	m_deviceMemory = new DeviceMemory(device, static_cast<uint32_t>(bufferMemoryRequirements.size),
	                                 device->FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	m_allocationSize = static_cast<uint32_t>(bufferMemoryRequirements.size);
	m_memoryOffset   = 0;

	m_device->Validate(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_deviceMemory->GetMemory(), m_memoryOffset));
}

Buffer::~Buffer()
{
	vkDestroyBuffer(m_device->GetDevice(), m_buffer, nullptr);
	if (m_memoryHeap == nullptr)
	{
		delete m_deviceMemory;
	}
}

VkBuffer& Buffer::GetBuffer() { return m_buffer; }

uint32_t Buffer::GetMemoryOffset() const { return m_memoryOffset; }

uint32_t Buffer::GetAllocationSize() const { return m_allocationSize; }

uint32_t Buffer::GetBufferSize() const { return m_bufferSize; }

MemoryHeap* Buffer::GetMemoryHeap() const { return m_memoryHeap; }

DeviceMemory* Buffer::GetDeviceMemory() const { return m_deviceMemory; }

VkDescriptorBufferInfo Buffer::GetDescriptorInfo() const
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer                 = m_buffer;
	bufferInfo.offset                 = 0;
	bufferInfo.range                  = m_bufferSize;

	return bufferInfo;
}

void Buffer::TransferInstantly(void* ptr, uint32_t size, uint32_t offset) const
{
	void* memoryPtr = nullptr;
	GetDeviceMemory()->Map(size, m_memoryOffset + offset, memoryPtr);
	memcpy(memoryPtr, ptr, size);
	GetDeviceMemory()->Unmap();
}

VkBuffer Buffer::CreateStaging() const
{
	VkBuffer staging = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &staging));
	return staging;
}

VkBuffer Buffer::CreateStaging(unsigned int bufferSize) const
{
	VkBuffer staging = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = bufferSize;
	bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &staging));
	return staging;
}

