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

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class Buffer;
class MemoryHeap;

class Texture
{
public:
	Texture(RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format,
	        VkImageUsageFlags imageUsageFlags, char* data = nullptr);

	Texture(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags,
	        char* data = nullptr);

	Texture(RenderDevice* device, MemoryHeap* memoryHeap, const VkImageCreateInfo& imageCreateInfo, char* data = nullptr);
	~Texture();

	void CopyBufferRegionsToImage(Buffer* buffer, VkBufferImageCopy* copies, uint32_t count);
	void TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout);

	VkDescriptorImageInfo GetDescriptorImageInfo();
	VkImage               GetImage() const;
	VkSampler             GetSampler() const;
	VkImageView           GetImageView() const;
	VkFormat              GetFormat() const;
	uint32_t              GetWidth() const;
	uint32_t              GetHeight() const;

private:
	void CreateImageFrom(const VkImageCreateInfo& imageCreateInfo);
	void TransferData(const VkImageCreateInfo& imageCreateInfo, char* data);
	void CreateSampler(const VkImageCreateInfo& imageCreateInfo);
	void CreateImageView(const VkImageCreateInfo& imageCreateInfo);

	void SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
	                    VkImageSubresourceRange subresourceRange);

	RenderDevice* m_device;

	bool         m_ownMemory;
	MemoryHeap*  m_memoryHeap;
	VkDeviceSize m_imageSize;
	VkDeviceSize m_memoryOffset;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;
	uint32_t m_layers;
	uint32_t m_mips;

	VkImage     m_image;
	VkSampler   m_sampler;
	VkImageView m_imageView;
	VkFormat    m_format;

	VkImageUsageFlags m_imageUsageFlags;
};

