#include "texture.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "devicememory.hpp"
#include "memoryheap.hpp"

Texture::Texture( RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags, char* data ) :
	mDevice( device ), mWidth( width ), mHeight( height ), mImageUsageFlags( imageUsageFlags ), mFormat( format ), mMemoryHeap( memoryHeap ), mOwnMemory(false)
{
	CreateImage( imageUsageFlags, data );
}

Texture::Texture( RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags, char* data ) :
	mDevice( device ), mWidth( width ), mHeight( height ), mImageUsageFlags( imageUsageFlags ), mFormat( format ), mOwnMemory( true )
{
	CreateImage( imageUsageFlags, data );
}

Texture::~Texture( )
{
	if ( mOwnMemory )
	{
		delete mMemoryHeap;
	}
	if ( mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT )
	{
		vkDestroySampler(
			mDevice->GetDevice( ),
			mSampler,
			nullptr
		);
	}
	vkDestroyImageView(
		mDevice->GetDevice( ),
		mImageView,
		nullptr
	);
	vkDestroyImage(
		mDevice->GetDevice( ),
		mImage,
		nullptr
	);
}

void Texture::TransitionImageLayout( VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout )
{
	mDevice->TransitionImageLayout(
		commandBuffer,
		mImage,
		mFormat,
		oldLayout,
		newLayout,
		{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1}
	);
}

VkDescriptorImageInfo Texture::GetDescriptorImageInfo( )
{
	VkDescriptorImageInfo descriptorImageInfo = {};
	// Provide the descriptor info the texture data it requires
	descriptorImageInfo.sampler = mSampler;
	descriptorImageInfo.imageView = mImageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return descriptorImageInfo;
}

void Texture::CreateImage( VkImageUsageFlags imageUsageFlags, char* data )
{
	VkDeviceSize texture_size = mWidth * mHeight * 4;

	std::unique_ptr<Buffer> transferBuffer;


	VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	if ( imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT )
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	}

	if ( data != nullptr )
	{
		transferBuffer = std::unique_ptr<Buffer>( new Buffer(
			mDevice, texture_size,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE
		) );

		transferBuffer->TransferInstantly( data, texture_size );
	}


	//////////////////////////////////
	////////////// Image /////////////
	//////////////////////////////////

	VkImageCreateInfo image_create_info = {};
	image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.extent.width = mWidth;
	image_create_info.extent.height = mHeight;
	image_create_info.extent.depth = 1;
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 1;
	image_create_info.format = mFormat;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	image_create_info.usage = mImageUsageFlags | (data != nullptr ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0);
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	mDevice->Validate( vkCreateImage(
		mDevice->GetDevice( ),
		&image_create_info,
		nullptr,
		&mImage
	) );

	VkMemoryRequirements imageMemoryRequirments;
	vkGetImageMemoryRequirements(
		mDevice->GetDevice( ),
		mImage,
		&imageMemoryRequirments
	);

	if ( mOwnMemory )
	{
		mMemoryHeap = new MemoryHeap( mDevice, imageMemoryRequirments.size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	uint32_t imageSize = (uint32_t) imageMemoryRequirments.size;
	uint32_t memoryOffset = mMemoryHeap->Allocate( imageSize, imageMemoryRequirments.alignment );


	mDevice->Validate( vkBindImageMemory(
		mDevice->GetDevice( ),
		mImage,
		mMemoryHeap->GetMemory()->GetMemory( ),
		memoryOffset
	) );


	VkCommandBuffer copy_cmd = mDevice->CreateSingleTimeCommand( );

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = aspectMask;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.layerCount = 1;



	if ( data != nullptr )
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = mWidth;
		bufferCopyRegion.imageExtent.height = mHeight;
		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = 0;
		SetImageLayout(
			copy_cmd,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			subresourceRange
		);
		vkCmdCopyBufferToImage(
			copy_cmd,
			transferBuffer->GetBuffer( ),
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferCopyRegion
		);
		SetImageLayout(
			copy_cmd,
			mImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			(mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
			subresourceRange
		);
	}
	else
	{

		SetImageLayout(
			copy_cmd,
			mImage,
			VK_IMAGE_LAYOUT_UNDEFINED,
			(mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
			subresourceRange
		);
	}


	mDevice->EndCommand( copy_cmd );

	////////////////////////////////////
	////////////// Sampler /////////////
	////////////////////////////////////
	if ( mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT )
	{
		VkSamplerCreateInfo samplerCreateInfo{};
		samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.maxAnisotropy = 1.0f;

		// Point
		samplerCreateInfo.magFilter = VK_FILTER_NEAREST;
		samplerCreateInfo.minFilter = VK_FILTER_NEAREST;
		// Byliniar
		//sampler_info.magFilter = VK_FILTER_LINEAR;
		//sampler_info.minFilter = VK_FILTER_LINEAR;
		samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias = 0.0f;
		samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod = 0.0f;
		// Set max level-of-detail to mip level count of the texture
		samplerCreateInfo.maxLod = (float) 1;

		// Do not use anisotropy
		samplerCreateInfo.maxAnisotropy = 1.0;
		samplerCreateInfo.anisotropyEnable = VK_FALSE;

		samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		mDevice->Validate( vkCreateSampler(
			mDevice->GetDevice( ),
			&samplerCreateInfo,
			nullptr,
			&mSampler
		) );
	}

	///////////////////////////////////////
	////////////// Image View /////////////
	///////////////////////////////////////


	
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image = mImage;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = mFormat;
	imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewCreateInfo.subresourceRange.aspectMask = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;

	mDevice->Validate( vkCreateImageView(
		mDevice->GetDevice( ),
		&imageViewCreateInfo,
		nullptr,
		&mImageView
	) );
}

void Texture::SetImageLayout( VkCommandBuffer cmdbuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange )
{
	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier{};
	imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange = subresourceRange;

	// Source layouts (old)
	// Source access mask controls actions that have to be finished on the old layout
	// before it will be transitioned to the new layout
	/*switch (oldImageLayout)
	{
	case VK_IMAGE_LAYOUT_UNDEFINED:
		// Image layout is undefined (or does not matter)
		// Only valid as initial layout
		// No flags required, listed only for completeness
		imageMemoryBarrier.srcAccessMask = 0;
		break;

	case VK_IMAGE_LAYOUT_PREINITIALIZED:
		// Image is preinitialized
		// Only valid as initial layout for linear images, preserves memory contents
		// Make sure host writes have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image is a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image is a depth/stencil attachment
		// Make sure any writes to the depth/stencil buffer have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image is a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image is a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image is read by a shader
		// Make sure any shader reads from the image have been finished
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}

	// Target layouts (new)
	// Destination access mask controls the dependency for the new image layout
	switch (newImageLayout)
	{
	case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		// Image will be used as a transfer destination
		// Make sure any writes to the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		// Image will be used as a transfer source
		// Make sure any reads from the image have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		break;

	case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
		// Image will be used as a color attachment
		// Make sure any writes to the color buffer have been finished
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		// Image layout will be used as a depth/stencil attachment
		// Make sure any writes to depth/stencil buffer have been finished
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		break;

	case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		// Image will be read in a shader (sampler, input attachment)
		// Make sure any writes to the image have been finished
		if (imageMemoryBarrier.srcAccessMask == 0)
		{
			imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		}
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		break;
	default:
		// Other source layouts aren't handled (yet)
		break;
	}*/


	if ( oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL )
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if ( oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL )
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		//throw std::invalid_argument("unsupported layout transition!");
	}





	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(
		cmdbuffer,
		srcStageMask,
		dstStageMask,
		0,
		0, nullptr,
		0, nullptr,
		1, &imageMemoryBarrier );
}
