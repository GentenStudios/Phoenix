#include <ResourceManager/globalresources.hpp>
#include <ResourceManager/resourcemanager.hpp>
#include <Renderer/device.hpp>
#include <Renderer/resourcetablelayout.hpp>
#include <Renderer/resourcetable.hpp>

#include <Globals/globals.hpp>

void CreateGlobalResources( RenderDevice* device, ResourceManager* resourceManager )
{
	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
				{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 ,VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 1 );
		resourceManager->RegisterResource<ResourceTableLayout>( "SettingsBufferResourceTableLayout", resourceTableLayout );

		// Create camera descriptor set instance
		ResourceTable* settingsResourceTable = resourceTableLayout->CreateTable();
		resourceManager->RegisterResource<ResourceTable>( "SettingsBufferResourceTable", settingsResourceTable );

	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			   { 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 ,VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_GEOMETRY_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 1 );
		resourceManager->RegisterResource<ResourceTableLayout>( "CameraResourceTableLayout", resourceTableLayout );

		// Create camera descriptor set instance
		ResourceTable* cameraResourceTable = resourceTableLayout->CreateTable( );
		resourceManager->RegisterResource<ResourceTable>( "CameraResourceTable", cameraResourceTable );
	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 ,VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 100 );
		resourceManager->RegisterResource<ResourceTableLayout>( "SamplerResourceTableLayout", resourceTableLayout );
	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, MAX_SPRITESHEET_SAMPLER_ARRAY ,VK_SHADER_STAGE_FRAGMENT_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 100 );
		resourceManager->RegisterResource<ResourceTableLayout>( "SamplerArrayResourceTableLayout", resourceTableLayout );

		ResourceTable* defaultSamplerArrayResourceTable = resourceTableLayout->CreateTable( );
		resourceManager->RegisterResource<ResourceTable>( "SamplerArrayResourceTable", defaultSamplerArrayResourceTable );
	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 ,VK_SHADER_STAGE_COMPUTE_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 100 );
		resourceManager->RegisterResource<ResourceTableLayout>( "ChunkPositionResourceTableLayout", resourceTableLayout );
	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			{ 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 ,VK_SHADER_STAGE_COMPUTE_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 100 );
		resourceManager->RegisterResource<ResourceTableLayout>( "IndexedIndirectCommandResourceTableLayout", resourceTableLayout );
	}

	{
		VkDescriptorSetLayoutBinding descriptorPoolSizes[] = {
			   { 0, VkDescriptorType::VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 , VK_SHADER_STAGE_COMPUTE_BIT }
		};
		ResourceTableLayout* resourceTableLayout = new ResourceTableLayout( device, descriptorPoolSizes, 1, 1 );
		resourceManager->RegisterResource<ResourceTableLayout>( "EngineSettingsResourceTableLayout", resourceTableLayout );
	}
}
