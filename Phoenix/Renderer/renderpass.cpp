#include "renderpass.hpp"
#include "device.hpp"
#include "framebufferattachment.hpp"

#include <assert.h>
#include <string.h>

RenderPass::RenderPass( RenderDevice* device, uint32_t width, uint32_t height, FramebufferAttachment* framebufferAttachment ) :
	mDevice( device ), mFramebufferAttachment( framebufferAttachment ), mWidth( width ), mHeight( height )
{

	std::vector<VkAttachmentDescription> colorAttachments;
	std::vector<VkAttachmentReference> colorAttachmentReferences;
	VkAttachmentReference depthAttachmentRefrence = {};
	bool usesDepth = false;

	for ( uint32_t i = 0 ; i < mFramebufferAttachment->GetFramebufferPackets( ).size(); i++ )
	{
		FramebufferPacket* packet = mFramebufferAttachment->GetFramebufferPackets( )[i];

		if ( packet->GetImageType( ) == EFramebufferImageType::Color )
		{
			VkAttachmentDescription colorAttachment = {};
			colorAttachment.format = packet->GetFormat( );
			colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			colorAttachment.finalLayout = packet->GetImageLayout( );

			colorAttachments.push_back( colorAttachment );
			// Present
			//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			// Color
			//colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			
			VkAttachmentReference colorAttachmentRefrence = {};
			colorAttachmentRefrence.attachment = i;
			colorAttachmentRefrence.layout = packet->GetImageLayout();

			colorAttachmentReferences.push_back( colorAttachmentRefrence );
		}
		else
		{
			VkAttachmentDescription depthAttachment = {};
			depthAttachment.format = packet->GetFormat( );
			depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			depthAttachment.finalLayout = packet->GetImageLayout( );

			colorAttachments.push_back( depthAttachment );


			depthAttachmentRefrence.attachment = i;
			depthAttachmentRefrence.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			usesDepth = true;
		}


	}


	mPipelineColorBlendAttachmentStateCount = 1;
	mPipelineColorBlendAttachmentStates = new VkPipelineColorBlendAttachmentState[mPipelineColorBlendAttachmentStateCount];
	{
		mPipelineColorBlendAttachmentStates[0] = {};
		mPipelineColorBlendAttachmentStates[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		mPipelineColorBlendAttachmentStates[0].blendEnable = VK_TRUE;
		mPipelineColorBlendAttachmentStates[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		mPipelineColorBlendAttachmentStates[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mPipelineColorBlendAttachmentStates[0].colorBlendOp = VK_BLEND_OP_ADD;
		mPipelineColorBlendAttachmentStates[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		mPipelineColorBlendAttachmentStates[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		mPipelineColorBlendAttachmentStates[0].alphaBlendOp = VK_BLEND_OP_MAX;
	}







	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentReferences.size( ));
	subpass.pColorAttachments = colorAttachmentReferences.data();
	subpass.pDepthStencilAttachment = usesDepth ? &depthAttachmentRefrence : nullptr;

	VkSubpassDependency subpassDependency = {};
	subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpassDependency.dstSubpass = 0;
	subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpassDependency.srcAccessMask = 0;
	subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	subpassDependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = static_cast<uint32_t>(colorAttachments.size());
	renderPassCreateInfo.pAttachments = colorAttachments.data();
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;
	renderPassCreateInfo.dependencyCount = 1;
	renderPassCreateInfo.pDependencies = &subpassDependency;

	mDevice->Validate(vkCreateRenderPass(
		mDevice->GetDevice(),
		&renderPassCreateInfo,
		nullptr,
		&mRenderPass
	));

	CreateFrameBuffer( width, height );
}

RenderPass::~RenderPass( )
{
	DestroyFrameBuffer( );

	delete[] mPipelineColorBlendAttachmentStates;

	vkDestroyRenderPass(
		mDevice->GetDevice( ),
		mRenderPass,
		nullptr
	);
}

void RenderPass::Use( VkCommandBuffer* commandBuffer, uint32_t index )
{
	float clearColorImage[4] = { 0.0f,0.0f,0.0f,1.0f };
	//float clearColorImage[4] = { 1.0f,1.0f,1.0f,1.0f };
	float clearColorPresent[4] = { 1.0f,1.0f,1.0f,1.0f };

	VkClearValue clearValues[3]{};

	memcpy( &clearValues[0].color.float32, clearColorImage, sizeof( float ) * 4 ); // Present
	memcpy( &clearValues[1].color.float32, clearColorPresent, sizeof( float ) * 4 ); // Color Image
	clearValues[2].depthStencil = {1.0f, 0};


	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = mRenderPass;
	renderPassInfo.renderArea.offset = {0, 0};
	renderPassInfo.renderArea.extent = {mWidth, mHeight};
	renderPassInfo.clearValueCount = 3;
	renderPassInfo.pClearValues = clearValues;

	renderPassInfo.framebuffer = mFramebuffers.get()[index];

	vkCmdBeginRenderPass(
		commandBuffer[index],
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);
}


void RenderPass::Rebuild( FramebufferAttachment* framebufferAttachment, uint32_t width, uint32_t height )
{
	mWidth = width;
	mHeight = height;
	mFramebufferAttachment = framebufferAttachment;
	DestroyFrameBuffer( );
	CreateFrameBuffer( width, height );
}

void RenderPass::CreateFrameBuffer( uint32_t width, uint32_t height )
{
	////////////////////////////////////////////////////////////////
	//////////////// Frame Buffers /////////////////////////////////
	////////////////////////////////////////////////////////////////

	mFramebuffers = std::unique_ptr<VkFramebuffer>( new VkFramebuffer[mDevice->GetSwapchainImageCount( )] );

	for ( uint32_t i = 0; i < mDevice->GetSwapchainImageCount( ); i++ )
	{
		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = mRenderPass;
		framebuffer_info.attachmentCount = mFramebufferAttachment->GetFramebufferAttachmentCount( );
		framebuffer_info.pAttachments = mFramebufferAttachment->GetFramebufferAttachments( i );
		framebuffer_info.width = width;
		framebuffer_info.height = height;
		framebuffer_info.layers = 1;

		mDevice->Validate( vkCreateFramebuffer(
			mDevice->GetDevice( ),
			&framebuffer_info,
			nullptr,
			&mFramebuffers.get( )[i]
		) );
	}
}

void RenderPass::DestroyFrameBuffer( )
{
	for ( uint32_t i = 0; i < mDevice->GetSwapchainImageCount( ); i++ )
	{
		vkDestroyFramebuffer(
			mDevice->GetDevice( ),
			mFramebuffers.get( )[i],
			nullptr
		);
	}
}