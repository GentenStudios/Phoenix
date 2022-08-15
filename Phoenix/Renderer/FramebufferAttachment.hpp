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
#include <vector>

class RenderDevice;
class DeviceMemory;
class Texture;

// struct SFrameBufferAttachmentInstance
//{
//	VkImage image;
//	VkDeviceMemory memory;
//	VkImageView view;
//	VkFormat format;
//	VkSampler sampler;
// };

enum class EFramebufferImageType
{
	Color,
	Depth
};

class FramebufferPacket
{
public:
	FramebufferPacket(Texture* textures, uint32_t textureCount, VkFormat format, VkImageLayout layout, EFramebufferImageType imageType);
	FramebufferPacket(VkImageView* imageViews, uint32_t textureCount, VkFormat format, VkImageLayout layout,
	                  EFramebufferImageType imageType);
	~FramebufferPacket();

	VkImageView*          GetImageViews() const;
	uint32_t              GetImageViewCount() const;
	VkFormat              GetFormat() const;
	VkImageLayout         GetImageLayout() const;
	EFramebufferImageType GetImageType() const;

private:
	std::unique_ptr<VkImageView> m_imageViews;
	uint32_t                     m_imageViewCount;
	VkFormat                     m_format;
	VkImageLayout                m_layout;
	EFramebufferImageType        m_imageType;
};

class FramebufferAttachment
{
public:
	FramebufferAttachment(RenderDevice* device, std::vector<FramebufferPacket*> framebufferPackets);
	~FramebufferAttachment();

	VkImageView* GetFramebufferAttachments(unsigned int swapchainImageIndex) const;
	uint32_t     GetFramebufferAttachmentCount() const;

	std::vector<FramebufferPacket*> GetFramebufferPackets() { return m_framebufferPackets; }

private:
	RenderDevice* m_device;
	uint32_t      m_framebufferAttachmentCount;
	uint32_t      m_framebufferCount;

	VkImageView**                   m_framebufferAttachments;
	std::vector<FramebufferPacket*> m_framebufferPackets;
};

