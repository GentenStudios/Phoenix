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
#include <Renderer/FramebufferAttachment.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/RenderTarget.hpp>
#include <Renderer/Renderpass.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Texture.hpp>

RenderTarget::RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, bool useDepth) : m_device(device), m_useDepth(useDepth)
{
	m_format = m_device->GetSurfaceFormat();
	CreateRenderTarget(width, height);
	m_renderpass = std::unique_ptr<RenderPass>(new RenderPass(m_device, width, height, m_framebufferAttachment.get()));
}

RenderTarget::RenderTarget(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, bool useDepth)
    : m_device(device), m_format(format), m_useDepth(useDepth)
{
	CreateRenderTarget(width, height);
	m_renderpass = std::unique_ptr<RenderPass>(new RenderPass(m_device, width, height, m_framebufferAttachment.get()));
}

RenderTarget::~RenderTarget()
{
	DestroyRenderTarget();
	m_renderpass.reset();
	m_samplerResourceTable.reset();
	m_depthImage.reset();
	m_depthFramebufferPacket.reset();
}

RenderPass* RenderTarget::GetRenderPass() const { return m_renderpass.get(); }

Texture* RenderTarget::GetImage() const { return m_image.get(); }

void RenderTarget::ScreenResize(uint32_t width, uint32_t height)
{
	DestroyRenderTarget();
	CreateRenderTarget(width, height);
	m_renderpass->Rebuild(m_framebufferAttachment.get(), width, height);
}

void RenderTarget::StartRendering(VkCommandBuffer* commandBuffer, uint32_t index) const
{
	VkViewport viewport = {};
	viewport.x          = 0;
	viewport.y          = 0;
	viewport.width      = (float) m_image->GetWidth();
	viewport.height     = (float) m_image->GetHeight();
	viewport.minDepth   = 0.0f;
	viewport.maxDepth   = 1.0f;

	VkRect2D scissor {};
	scissor.extent.width  = m_image->GetWidth();
	scissor.extent.height = m_image->GetHeight();
	scissor.offset.x      = 0;
	scissor.offset.y      = 0;

	m_renderpass->Use(commandBuffer, index);

	vkCmdSetViewport(commandBuffer[index], 0, 1, &viewport);

	vkCmdSetScissor(commandBuffer[index], 0, 1, &scissor);
}

void RenderTarget::StartSampling(VkCommandBuffer* commandBuffer, uint32_t index, Pipeline* pipeline) const
{
	m_image->TransitionImageLayout(commandBuffer[index], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	m_samplerResourceTable->Use(commandBuffer, index, 0, pipeline->GetPipelineLayout());
}

void RenderTarget::StopSampling(VkCommandBuffer* commandBuffer, uint32_t index) const
{
	m_image->TransitionImageLayout(commandBuffer[index], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
}

void RenderTarget::CreateRenderTarget(uint32_t width, uint32_t height)
{
	m_image = std::make_unique<Texture>(m_device, width, height, m_format,
	                                    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT |
	                                    VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
	                                    nullptr);

	m_framebufferPacket = std::make_unique<FramebufferPacket>(m_image.get(), 1, m_format, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	                                                          EFramebufferImageType::Color);

	if (m_useDepth)
	{
		m_depthImage = std::make_unique<Texture>(m_device, width, height, m_device->GetDepthFormat(),
		                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
		                                         nullptr);

		m_depthFramebufferPacket = std::make_unique<FramebufferPacket>(
			m_depthImage.get(), 1, VK_FORMAT_D32_SFLOAT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, EFramebufferImageType::Depth);

		m_framebufferAttachment = std::unique_ptr<FramebufferAttachment>(
		    new FramebufferAttachment(m_device, {m_framebufferPacket.get(), m_depthFramebufferPacket.get()}));
	}
	else
	{
		m_framebufferAttachment = std::unique_ptr<FramebufferAttachment>(new FramebufferAttachment(m_device, {m_framebufferPacket.get()}));
	}

	if (m_samplerResourceTable == nullptr)
	{
		m_samplerResourceTable = std::unique_ptr<ResourceTable>(m_device->GetPostProcessSampler()->CreateTable());
	}

	m_samplerResourceTable->Bind(0, m_image.get());
}

void RenderTarget::DestroyRenderTarget()
{
	m_image.reset();
	m_framebufferAttachment.reset();
	m_framebufferPacket.reset();
}

