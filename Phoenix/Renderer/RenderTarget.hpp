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
class RenderPass;
class Texture;
class FramebufferAttachment;
class FramebufferPacket;
class ResourceTableLayout;
class ResourceTable;
class Pipeline;

class RenderTarget
{
public:
	RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, bool useDepth = false);
	RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, bool useDepth = false);
	~RenderTarget();

	RenderPass* GetRenderPass() const;
	Texture*    GetImage() const;

	void ScreenResize(uint32_t width, uint32_t height);

	void StartRendering(VkCommandBuffer* commandBuffer, uint32_t index) const;

	void StartSampling(VkCommandBuffer* commandBuffer, uint32_t index, Pipeline* pipeline) const;

	void StopSampling(VkCommandBuffer* commandBuffer, uint32_t index) const;

private:
	void CreateRenderTarget(uint32_t width, uint32_t height);
	void DestroyRenderTarget();

	RenderDevice*                          m_device;
	std::unique_ptr<RenderPass>            m_renderpass;
	std::unique_ptr<Texture>               m_image;
	std::unique_ptr<Texture>               m_depthImage;
	std::unique_ptr<FramebufferPacket>     m_framebufferPacket;
	std::unique_ptr<FramebufferPacket>     m_depthFramebufferPacket;
	std::unique_ptr<FramebufferAttachment> m_framebufferAttachment;
	std::unique_ptr<ResourceTable>         m_samplerResourceTable;
	VkFormat                               m_format;
	bool                                   m_useDepth;
};

