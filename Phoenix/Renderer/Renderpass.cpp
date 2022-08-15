#include <Renderer/Device.hpp>
#include <Renderer/FramebufferAttachment.hpp>
#include <Renderer/Renderpass.hpp>

#include <cassert>
#include <cstring>

RenderPass::RenderPass(RenderDevice* device, uint32_t width, uint32_t height, FramebufferAttachment* framebufferAttachment)
    : m_device(device), m_width(width), m_height(height), m_framebufferAttachment(framebufferAttachment)
{
	std::vector<VkAttachmentDescription> colorAttachments;
	std::vector<VkAttachmentReference>   colorAttachmentReferences;
	VkAttachmentReference                depthAttachmentReference = {};
	bool                                 usesDepth                = false;

	for (uint32_t i = 0; i < m_framebufferAttachment->GetFramebufferPackets().size(); i++)
	{
		FramebufferPacket* packet = m_framebufferAttachment->GetFramebufferPackets()[i];

		if (packet->GetImageType() == EFramebufferImageType::Color)
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format                  = packet->GetFormat();
			colorAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout             = packet->GetImageLayout();

			colorAttachments.push_back(colorAttachment);

			// Present
			// colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Color
			// colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorAttachmentReference = {};
			colorAttachmentReference.attachment            = i;
			colorAttachmentReference.layout                = packet->GetImageLayout();

			colorAttachmentReferences.push_back(colorAttachmentReference);
		}
		else
		{
			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format                  = packet->GetFormat();
			depthAttachment.samples                 = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp                  = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp                 = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp           = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp          = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout           = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout             = packet->GetImageLayout();

			colorAttachments.push_back(depthAttachment);

			depthAttachmentReference.attachment = i;
			depthAttachmentReference.layout     = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			usesDepth                          = true;
		}
	}

	m_pipelineColorBlendAttachmentStateCount = 1;
	m_pipelineColorBlendAttachmentStates =
	    std::make_unique<VkPipelineColorBlendAttachmentState[]>(m_pipelineColorBlendAttachmentStateCount);

	{
		m_pipelineColorBlendAttachmentStates[0] = {};
		m_pipelineColorBlendAttachmentStates[0].colorWriteMask =
		    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		m_pipelineColorBlendAttachmentStates[0].blendEnable         = VK_TRUE;
		m_pipelineColorBlendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		m_pipelineColorBlendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_pipelineColorBlendAttachmentStates[0].colorBlendOp        = VK_BLEND_OP_ADD;
		m_pipelineColorBlendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		m_pipelineColorBlendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		m_pipelineColorBlendAttachmentStates[0].alphaBlendOp        = VK_BLEND_OP_MAX;
	}

	VkSubpassDescription subpass    = {};
	subpass.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount    = static_cast<uint32_t>(colorAttachmentReferences.size());
	subpass.pColorAttachments       = colorAttachmentReferences.data();
	subpass.pDepthStencilAttachment = usesDepth ? &depthAttachmentReference : nullptr;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass          = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass          = 0;
	subpassDependency.srcStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask        = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask       = 0;
	subpassDependency.dstAccessMask       = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags     = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount        = static_cast<uint32_t>(colorAttachments.size());
	renderPassCreateInfo.pAttachments           = colorAttachments.data();
	renderPassCreateInfo.subpassCount           = 1;
	renderPassCreateInfo.pSubpasses             = &subpass;
	renderPassCreateInfo.dependencyCount        = 1;
	renderPassCreateInfo.pDependencies          = &subpassDependency;

	m_device->Validate(vkCreateRenderPass(m_device->GetDevice(), &renderPassCreateInfo, nullptr, &m_renderpass));

	CreateFrameBuffer(width, height);
}

RenderPass::~RenderPass()
{
	DestroyFrameBuffer();
	vkDestroyRenderPass(m_device->GetDevice(), m_renderpass, nullptr);
}

void RenderPass::Use(VkCommandBuffer* commandBuffer, uint32_t index) const
{
	const float clearColorImage[4]   = {0.0f, 0.0f, 0.0f, 1.0f};
	const float clearColorPresent[4] = {1.0f, 1.0f, 1.0f, 1.0f};

	VkClearValue clearValues[3] {};

	memcpy(&clearValues[0].color.float32, clearColorImage, sizeof(float) * 4);   // Present
	memcpy(&clearValues[1].color.float32, clearColorPresent, sizeof(float) * 4); // Color Image
	clearValues[2].depthStencil = {1.0f, 0};

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass            = m_renderpass;
	renderPassInfo.renderArea.offset     = {0, 0};
	renderPassInfo.renderArea.extent     = {m_width, m_height};
	renderPassInfo.clearValueCount       = 3;
	renderPassInfo.pClearValues          = clearValues;

	renderPassInfo.framebuffer = m_framebuffers[index];

	vkCmdBeginRenderPass(commandBuffer[index], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

VkRenderPass RenderPass::GetRenderPass() const { return m_renderpass; }

VkFramebuffer* RenderPass::GetFrameBuffers() const { return m_framebuffers.get(); }

uint32_t RenderPass::GetPipelineColorBlendAttachmentStateCount() const { return m_pipelineColorBlendAttachmentStateCount; }

VkPipelineColorBlendAttachmentState* RenderPass::GetPipelineColorBlendAttachmentStates() const
{
	return m_pipelineColorBlendAttachmentStates.get();
}

void RenderPass::Rebuild(FramebufferAttachment* framebufferAttachment, uint32_t width, uint32_t height)
{
	m_width                 = width;
	m_height                = height;
	m_framebufferAttachment = framebufferAttachment;

	DestroyFrameBuffer();
	CreateFrameBuffer(width, height);
}

void RenderPass::CreateFrameBuffer(uint32_t width, uint32_t height)
{
	m_framebuffers = std::make_unique<VkFramebuffer[]>(m_device->GetSwapchainImageCount());

	for (uint32_t i = 0; i < m_device->GetSwapchainImageCount(); i++)
	{
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass              = m_renderpass;
		framebufferInfo.attachmentCount         = m_framebufferAttachment->GetFramebufferAttachmentCount();
		framebufferInfo.pAttachments            = m_framebufferAttachment->GetFramebufferAttachments(i);
		framebufferInfo.width                   = width;
		framebufferInfo.height                  = height;
		framebufferInfo.layers                  = 1;

		m_device->Validate(vkCreateFramebuffer(m_device->GetDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]));
	}
}

void RenderPass::DestroyFrameBuffer()
{
	for (uint32_t i = 0; i < m_device->GetSwapchainImageCount(); i++)
	{
		vkDestroyFramebuffer(m_device->GetDevice(), m_framebuffers[i], nullptr);
	}
}
