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

#pragma once

#include <Renderer/MemoryHeap.hpp>
#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;

class Buffer
{
public:
	Buffer(RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);
	Buffer(RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);
	~Buffer();

	VkBuffer&     GetBuffer();
	uint32_t      GetMemoryOffset() const;
	uint32_t      GetAllocationSize() const;
	uint32_t      GetBufferSize() const;
	MemoryHeap*   GetMemoryHeap() const;
	DeviceMemory* GetDeviceMemory() const;

	VkDescriptorBufferInfo GetDescriptorInfo() const;

	void TransferInstantly(void* ptr, uint32_t size, uint32_t offset = 0) const;

private:
	VkBuffer CreateStaging() const;
	VkBuffer CreateStaging(unsigned int bufferSize) const;

private:
	RenderDevice* m_device;

	MemoryHeap*   m_memoryHeap   = nullptr;
	DeviceMemory* m_deviceMemory = nullptr;
	uint32_t      m_memoryOffset;
	uint32_t      m_allocationSize;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	uint32_t m_bufferSize;
};

