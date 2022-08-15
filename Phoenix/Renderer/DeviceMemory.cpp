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
#include <Renderer/DeviceMemory.hpp>

DeviceMemory::DeviceMemory(RenderDevice* device, uint32_t size, uint32_t memoryProperties) : m_device(device), m_size(size)
{

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize       = size;
	memoryAllocateInfo.memoryTypeIndex      = memoryProperties;

	m_device->Validate(vkAllocateMemory(m_device->GetDevice(), &memoryAllocateInfo, nullptr, &m_memory));
}

DeviceMemory::~DeviceMemory() { vkFreeMemory(m_device->GetDevice(), m_memory, nullptr); }

VkDeviceMemory DeviceMemory::GetMemory() const { return m_memory; }
uint32_t       DeviceMemory::GetSize() const { return m_size; }

void DeviceMemory::Map(VkDeviceSize size, VkDeviceSize offset, void*& ptr)
{
	m_device->Validate(vkMapMemory(m_device->GetDevice(), m_memory, offset, size, 0, &ptr));
}

void DeviceMemory::Unmap() { vkUnmapMemory(m_device->GetDevice(), m_memory); }

