#include <Renderer/Device.hpp>
#include <Renderer/PipelineLayout.hpp>

PipelineLayout::PipelineLayout(RenderDevice* device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount)
    : m_device(device)
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount             = descriptorSetLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts                = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount     = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges        = 0;

	// Create the pipeline layout
	m_device->Validate(vkCreatePipelineLayout(m_device->GetDevice(), &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout));
}

PipelineLayout::~PipelineLayout() { vkDestroyPipelineLayout(m_device->GetDevice(), m_pipelineLayout, nullptr); }

VkPipelineLayout PipelineLayout::GetPipelineLayout() const { return m_pipelineLayout; }
