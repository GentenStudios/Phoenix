#include <Renderer/PipelineLayout.hpp>
#include <Renderer/Device.hpp>

PipelineLayout::PipelineLayout( RenderDevice* device, VkDescriptorSetLayout* descriptorSetLayouts, uint32_t descriptorSetLayoutCount ) : mDevice( device )
{
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = descriptorSetLayoutCount;
	pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts;
	pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
	pipelineLayoutCreateInfo.pPushConstantRanges = 0;

	// Create the pipeline layout
	mDevice->Validate( vkCreatePipelineLayout(
		mDevice->GetDevice(),
		&pipelineLayoutCreateInfo,
		nullptr,
		&mPipelineLayout
	) );
}

PipelineLayout::~PipelineLayout( )
{
	vkDestroyPipelineLayout(
		mDevice->GetDevice( ),
		mPipelineLayout,
		nullptr
	);
}
