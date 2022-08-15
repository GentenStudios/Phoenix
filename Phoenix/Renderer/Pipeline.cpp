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
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/Renderpass.hpp>

Pipeline::Pipeline(RenderDevice* device, PipelineType type, RenderPass* renderPass, PipelineLayout* pipelineLayout,
                   VkPipelineShaderStageCreateInfo* pipelineShaderStageInfos, uint32_t pipelineShaderStageInfoCount,
                   VkVertexInputBindingDescription* vertexInputBindingDescriptions, uint32_t vertexInputBindingDescriptionCount,
                   VkVertexInputAttributeDescription* vertexInputAttributeDescriptions, uint32_t vertexInputAttributeDescriptionCount,
                   VkPrimitiveTopology topology)
    : m_device(device), m_pipelineLayout(pipelineLayout), m_type(type)
{
	switch (type)
	{
	case PipelineType::Graphics:
	{
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType                                = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount        = vertexInputBindingDescriptionCount;
		vertexInputInfo.vertexAttributeDescriptionCount      = vertexInputAttributeDescriptionCount;
		vertexInputInfo.pVertexBindingDescriptions           = vertexInputBindingDescriptions;
		vertexInputInfo.pVertexAttributeDescriptions         = vertexInputAttributeDescriptions;

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
		graphicsPipelineCreateInfo.pVertexInputState            = &vertexInputInfo;
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

		m_device->Validate(
		    vkCreateGraphicsPipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_pipeline));

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

		m_device->Validate(
		    vkCreateComputePipelines(m_device->GetDevice(), VK_NULL_HANDLE, 1, &computePipelineCreateInfo, nullptr, &m_pipeline));
		break;
	}
	}
}

Pipeline::~Pipeline() { vkDestroyPipeline(m_device->GetDevice(), m_pipeline, nullptr); }

void Pipeline::Use(VkCommandBuffer* commandBuffer, uint32_t index) const
{
	VkPipelineBindPoint bindPoint = {};
	switch (m_type)
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

	vkCmdBindPipeline(commandBuffer[index], bindPoint, m_pipeline);
}

VkPipelineLayout Pipeline::GetPipelineLayout() const { return m_pipelineLayout->GetPipelineLayout(); }

