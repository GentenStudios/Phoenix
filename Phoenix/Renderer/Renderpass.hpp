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
class FramebufferAttachment;
class FramebufferPacket;

class RenderPass
{
public:
	RenderPass(RenderDevice* device, uint32_t width, uint32_t height, FramebufferAttachment* framebufferAttachment);
	~RenderPass();

	void Use(VkCommandBuffer* commandBuffer, uint32_t index) const;

	VkRenderPass   GetRenderPass() const;
	VkFramebuffer* GetFrameBuffers() const;

	uint32_t                             GetPipelineColorBlendAttachmentStateCount() const;
	VkPipelineColorBlendAttachmentState* GetPipelineColorBlendAttachmentStates() const;

	void Rebuild(FramebufferAttachment* framebufferAttachment, uint32_t width, uint32_t height);

private:
	void CreateFrameBuffer(uint32_t width, uint32_t height);
	void DestroyFrameBuffer();

private:
	RenderDevice*                                          m_device;
	uint32_t                                               m_width;
	uint32_t                                               m_height;
	FramebufferAttachment*                                 m_framebufferAttachment;
	VkRenderPass                                           m_renderpass = VK_NULL_HANDLE;
	uint32_t                                               m_pipelineColorBlendAttachmentStateCount;
	std::unique_ptr<VkPipelineColorBlendAttachmentState[]> m_pipelineColorBlendAttachmentStates;
	std::unique_ptr<VkFramebuffer[]>                       m_framebuffers;
};

