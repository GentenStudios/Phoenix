#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/ResourceTable.hpp>

#include <assert.h>
#include <string.h>

ResourceTableLayout::ResourceTableLayout( RenderDevice* device, VkDescriptorSetLayoutBinding* descriptorSetLayoutBinding, uint32_t descriptorSetLayoutBindingCount, uint32_t maxSets ) : mDevice( device )
{
	mDescriptorSetLayoutBindings = std::unique_ptr<VkDescriptorSetLayoutBinding>( new VkDescriptorSetLayoutBinding[descriptorSetLayoutBindingCount] );
	memcpy( mDescriptorSetLayoutBindings.get( ), descriptorSetLayoutBinding, sizeof( VkDescriptorSetLayoutBinding ) * descriptorSetLayoutBindingCount );
	mDescriptorSetLayoutBindingCount = descriptorSetLayoutBindingCount;


	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {};
	descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutCreateInfo.bindingCount = descriptorSetLayoutBindingCount;
	descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBinding;

	mDevice->Validate( vkCreateDescriptorSetLayout(
		mDevice->GetDevice( ),
		&descriptorSetLayoutCreateInfo,
		nullptr,
		&mDescriptorSetLayout
	) );

	std::unique_ptr<VkDescriptorPoolSize> descriptorPoolSizes( new VkDescriptorPoolSize[descriptorSetLayoutBindingCount] );

	for ( uint32_t i = 0; i < descriptorSetLayoutBindingCount; ++i )
	{
		descriptorPoolSizes.get()[i].type = descriptorSetLayoutBinding[i].descriptorType;
		descriptorPoolSizes.get()[i].descriptorCount = maxSets;
	}

	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {};
	descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolCreateInfo.poolSizeCount = descriptorSetLayoutBindingCount;
	descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.get();
	descriptorPoolCreateInfo.maxSets = maxSets;

	mDevice->Validate( vkCreateDescriptorPool(
		mDevice->GetDevice(),
		&descriptorPoolCreateInfo,
		nullptr,
		&mDescriptorPool
	) );
}

ResourceTableLayout::~ResourceTableLayout( )
{
	vkDestroyDescriptorSetLayout(
		mDevice->GetDevice( ),
		mDescriptorSetLayout,
		nullptr
	);
	vkDestroyDescriptorPool(
		mDevice->GetDevice( ),
		mDescriptorPool,
		nullptr
	);
}

ResourceTable* ResourceTableLayout::CreateTable( )
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {};
	descriptorSetAllocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	descriptorSetAllocateInfo.descriptorPool = mDescriptorPool;
	descriptorSetAllocateInfo.descriptorSetCount = 1;
	descriptorSetAllocateInfo.pSetLayouts = &mDescriptorSetLayout;

	VkDescriptorSet descriptorSet = VK_NULL_HANDLE;

	mDevice->Validate( vkAllocateDescriptorSets(
		mDevice->GetDevice( ),
		&descriptorSetAllocateInfo,
		&descriptorSet
	) );

	return new ResourceTable( mDevice, this, descriptorSet );
}

VkDescriptorType ResourceTableLayout::GetDescriptorType( uint32_t index )
{
	assert( index < mDescriptorSetLayoutBindingCount );
	return mDescriptorSetLayoutBindings.get()[index].descriptorType;
}