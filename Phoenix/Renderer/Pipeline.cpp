#include <Renderer/Device.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/Renderpass.hpp>

Pipeline::Pipeline(RenderDevice* device, PipelineType type, RenderPass* renderPass, PipelineLayout* pipelineLayout,
                   VkPipelineShaderStageCreateInfo* pipelineShaderStageInfos, uint32_t pipelineShaderStageInfoCount,
                   VkVertexInputBindingDescription* vertexInputBindingDescriptions, uint32_t vertexInputBindingDescriptionCount,
                   VkVertexInputAttributeDescription* vertexInputAttributeDescriptions, uint32_t vertexInputAttributeDescriptionCount,
                   VkPrimitiveTopology topology)
    : mDevice(device), mPipelineLayout(pipelineLayout), mType(type)
{

	switch (type)
	{
	case PipelineType::Graphics:
	{
		VkPipelineVertexInputStateCreateInfo vertex_input_info = {};
		vertex_input_info.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexBindingDescriptionCount        = vertexInputBindingDescriptionCount;
		vertex_input_info.vertexAttributeDescriptionCount      = vertexInputAttributeDescriptionCount;
		vertex_input_info.pVertexBindingDescriptions           = vertexInputBindingDescriptions;
		vertex_input_info.pVertexAttributeDescriptions         = vertexInputAttributeDescriptions;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology                               = topology;
		inputAssembly.primitiveRestartEnable                 = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType                             = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount                     = 1;
		viewportState.scissorCount                      = 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType                                  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable                       = VK_FALSE;
		rasterizer.rasterizerDiscardEnable                = VK_FALSE;
		rasterizer.polygonMode                            = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth                              = 1.0f;
		rasterizer.cullMode                               = VK_CULL_MODE_FRONT_BIT;
		rasterizer.frontFace                              = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizer.depthBiasEnable                        = VK_FALSE;
		rasterizer.depthBiasConstantFactor                = 0.0f;
		rasterizer.depthBiasClamp                         = 0.0f;
		rasterizer.depthBiasSlopeFactor                   = 0.0f;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType                                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable                  = VK_FALSE;
		multisampling.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading                     = 1.0f;
		multisampling.pSampleMask                          = nullptr;
		multisampling.alphaToCoverageEnable                = VK_FALSE;
		multisampling.alphaToOneEnable                     = VK_FALSE;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType                                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable                       = VK_TRUE;
		depthStencil.depthWriteEnable                      = VK_TRUE;
		depthStencil.depthCompareOp                        = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencil.depthBoundsTestEnable                 = VK_FALSE;
		depthStencil.stencilTestEnable                     = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType                               = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable                       = VK_FALSE;
		colorBlending.logicOp                             = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount                     = renderPass->GetPipelineColorBlendAttachmentStateCount();
		colorBlending.pAttachments                        = renderPass->GetPipelineColorBlendAttachmentStates();
		colorBlending.blendConstants[0]                   = 0.0f;
		colorBlending.blendConstants[1]                   = 0.0f;
		colorBlending.blendConstants[2]                   = 0.0f;
		colorBlending.blendConstants[3]                   = 0.0f;

		const uint32_t dynamicStateCount                = 3;
		VkDynamicState dynamicStates[dynamicStateCount] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR,
		                                                   VK_DYNAMIC_STATE_LINE_WIDTH};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType                            = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pDynamicStates                   = dynamicStates;
		dynamicState.dynamicStateCount                = dynamicStateCount;
		dynamicState.flags                            = 0;

		VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
		graphicsPipelineCreateInfo.sType                        = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCreateInfo.stageCount                   = pipelineShaderStageInfoCount;
		graphicsPipelineCreateInfo.pStages                      = pipelineShaderStageInfos;
		graphicsPipelineCreateInfo.pVertexInputState            = &vertex_input_info;
		graphicsPipelineCreateInfo.pInputAssemblyState          = &inputAssembly;
		graphicsPipelineCreateInfo.pViewportState               = &viewportState;
		graphicsPipelineCreateInfo.pRasterizationState          = &rasterizer;
		graphicsPipelineCreateInfo.pMultisampleState            = &multisampling;
		graphicsPipelineCreateInfo.pDepthStencilState           = &depthStencil;
		graphicsPipelineCreateInfo.pColorBlendState             = &colorBlending;
		graphicsPipelineCreateInfo.pDynamicState                = &dynamicState;
		graphicsPipelineCreateInfo.layout                       = pipelineLayout->GetPipelineLayout();
		graphicsPipelineCreateInfo.renderPass                   = renderPass->GetRenderPass();
		graphicsPipelineCreateInfo.subpass                      = 0;
		graphicsPipelineCreateInfo.basePipelineHandle           = VK_NULL_HANDLE;
		graphicsPipelineCreateInfo.basePipelineIndex            = -1;
		graphicsPipelineCreateInfo.flags                        = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;

		mDevice->Validate(
		    vkCreateGraphicsPipelines(mDevice->GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &mPipeline));

		break;
	}
	case PipelineType::Compute:
	{

		VkComputePipelineCreateInfo computePipelineCreateInfo = {};
		computePipelineCreateInfo.sType                       = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
		computePipelineCreateInfo.pNext                       = nullptr;
		computePipelineCreateInfo.flags                       = 0;
		computePipelineCreateInfo.stage                       = *pipelineShaderStageInfos;
		computePipelineCreateInfo.layout                      = pipelineLayout->GetPipelineLayout();
		computePipelineCreateInfo.basePipelineHandle          = VK_NULL_HANDLE;
		computePipelineCreateInfo.basePipelineIndex           = -1;

		mDevice->Validate(
		    vkCreateComputePipelines(mDevice->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &mPipeline));
		break;
	}
	}
}

Pipeline::~Pipeline() { vkDestroyPipeline(mDevice->GetDevice(), mPipeline, nullptr); }

void Pipeline::Use(VkCommandBuffer* commandBuffer, uint32_t index)
{
	VkPipelineBindPoint bindPoint;
	switch (mType)
	{
	case PipelineType::Graphics:
	{
		bindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		break;
	}
	case PipelineType::Compute:
	{
		bindPoint = VK_PIPELINE_BIND_POINT_COMPUTE;
		break;
	}
	}
	vkCmdBindPipeline(commandBuffer[index], bindPoint, mPipeline);
}

VkPipelineLayout Pipeline::GetPipelineLayout() { return mPipelineLayout->GetPipelineLayout(); }
