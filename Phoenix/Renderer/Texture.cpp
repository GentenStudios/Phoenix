#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Renderer/Texture.hpp>

Texture::Texture(RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format,
                 VkImageUsageFlags imageUsageFlags, char* data)
    : mDevice(device), mMemoryHeap(memoryHeap), mWidth(width), mHeight(height), mFormat(format), mImageUsageFlags(imageUsageFlags)
{
	if (mMemoryHeap == nullptr)
	{
		mOwnMemory = true;
	}

	// Default to 2D texture with 1 layer and 1 mipmap level.
	mDepth  = 1;
	mLayers = 1;
	mMips   = 1;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width      = mWidth;
	imageCreateInfo.extent.height     = mHeight;
	imageCreateInfo.extent.depth      = mDepth;
	imageCreateInfo.mipLevels         = mMips;
	imageCreateInfo.arrayLayers       = mLayers;
	imageCreateInfo.format            = mFormat;
	imageCreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage             = mImageUsageFlags | (data != nullptr ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0);
	imageCreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;

	CreateImageFrom(imageCreateInfo);
	TransferData(imageCreateInfo, data);
	CreateSampler(imageCreateInfo);
	CreateImageView(imageCreateInfo);
}

Texture::Texture(RenderDevice* device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags imageUsageFlags, char* data)
    : Texture(device, nullptr, width, height, format, imageUsageFlags, data)
{
}

Texture::Texture(RenderDevice* device, MemoryHeap* memoryHeap, const VkImageCreateInfo& imageCreateInfo, char* data) : mDevice(device), mMemoryHeap(memoryHeap)
{
	if (mMemoryHeap == nullptr)
	{
		mOwnMemory = true;
	}

	mWidth           = imageCreateInfo.extent.width;
	mHeight          = imageCreateInfo.extent.height;
	mDepth           = imageCreateInfo.extent.depth;
	mLayers          = imageCreateInfo.arrayLayers;
	mMips            = imageCreateInfo.mipLevels;
	mFormat          = imageCreateInfo.format;
	mImageUsageFlags = imageCreateInfo.usage;

	CreateImageFrom(imageCreateInfo);
	TransferData(imageCreateInfo, data);
	CreateSampler(imageCreateInfo);
	CreateImageView(imageCreateInfo);
}

Texture::~Texture()
{
	if (mOwnMemory)
	{
		delete mMemoryHeap;
	}

	if (mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		vkDestroySampler(mDevice->GetDevice(), mSampler, nullptr);
	}

	vkDestroyImageView(mDevice->GetDevice(), mImageView, nullptr);
	vkDestroyImage(mDevice->GetDevice(), mImage, nullptr);
}

void Texture::CopyBufferRegionsToImage(Buffer* buffer, VkBufferImageCopy* copies, uint32_t count)
{
	VkCommandBuffer copyCmd = mDevice->CreateSingleTimeCommand();

	const VkImageAspectFlagBits aspectMask =
	    mImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask              = aspectMask;
	subresourceRange.baseMipLevel            = 0;
	subresourceRange.levelCount              = mMips;
	subresourceRange.layerCount              = mLayers;

	SetImageLayout(copyCmd, mImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	vkCmdCopyBufferToImage(copyCmd, buffer->GetBuffer(), mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, count, copies);
	SetImageLayout(copyCmd, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	               (mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
	               subresourceRange);

	mDevice->EndCommand(copyCmd);
}

void Texture::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	mDevice->TransitionImageLayout(commandBuffer, mImage, mFormat, oldLayout, newLayout, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
}

VkDescriptorImageInfo Texture::GetDescriptorImageInfo()
{
	VkDescriptorImageInfo descriptorImageInfo = {};
	// Provide the descriptor info the texture data it requires
	descriptorImageInfo.sampler     = mSampler;
	descriptorImageInfo.imageView   = mImageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return descriptorImageInfo;
}

void Texture::CreateImageFrom(const VkImageCreateInfo& imageCreateInfo)
{
	mDevice->Validate(vkCreateImage(mDevice->GetDevice(), &imageCreateInfo, nullptr, &mImage));

	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(mDevice->GetDevice(), mImage, &imageMemoryRequirements);

	if (mOwnMemory)
	{
		mMemoryHeap = new MemoryHeap(mDevice, imageMemoryRequirements.size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	mImageSize    = static_cast<uint32_t>(imageMemoryRequirements.size);
	mMemoryOffset = mMemoryHeap->Allocate(mImageSize, imageMemoryRequirements.alignment);

	mDevice->Validate(vkBindImageMemory(mDevice->GetDevice(), mImage, mMemoryHeap->GetMemory()->GetMemory(), mMemoryOffset));
}

void Texture::TransferData(const VkImageCreateInfo& imageCreateInfo, char* data)
{
	if (data == nullptr)
		return;

	std::unique_ptr<Buffer> transferBuffer =
	    std::unique_ptr<Buffer>(new Buffer(mDevice, mImageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE));

	transferBuffer->TransferInstantly(data, mImageSize);

	VkCommandBuffer copyCmd = mDevice->CreateSingleTimeCommand();

	const VkImageAspectFlagBits aspectMask =
	    mImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask              = aspectMask;
	subresourceRange.baseMipLevel            = 0;
	subresourceRange.levelCount              = mMips;
	subresourceRange.layerCount              = mLayers;

	if (data != nullptr)
	{
		VkBufferImageCopy bufferCopyRegion               = {};
		bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel       = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount     = mLayers;
		bufferCopyRegion.imageExtent.width               = mWidth;
		bufferCopyRegion.imageExtent.height              = mHeight;
		bufferCopyRegion.imageExtent.depth               = mDepth;
		bufferCopyRegion.bufferOffset                    = 0;
		SetImageLayout(copyCmd, mImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
		vkCmdCopyBufferToImage(copyCmd, transferBuffer->GetBuffer(), mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
		SetImageLayout(copyCmd, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		               (mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
		               subresourceRange);
	}
	else
	{

		SetImageLayout(copyCmd, mImage, VK_IMAGE_LAYOUT_UNDEFINED,
		               (mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
		               subresourceRange);
	}

	mDevice->EndCommand(copyCmd);
}

void Texture::CreateSampler(const VkImageCreateInfo& imageCreateInfo)
{
	if (mImageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		VkSamplerCreateInfo samplerCreateInfo = {};
		samplerCreateInfo.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCreateInfo.magFilter           = VK_FILTER_NEAREST; // Use VK_FILTER_LINEAR for bilinear filtering.
		samplerCreateInfo.minFilter           = VK_FILTER_NEAREST;
		samplerCreateInfo.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCreateInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerCreateInfo.mipLodBias          = 0.0f;
		samplerCreateInfo.compareOp           = VK_COMPARE_OP_NEVER;
		samplerCreateInfo.minLod              = 0.0f;
		samplerCreateInfo.maxLod              = static_cast<float>(mMips);
		samplerCreateInfo.anisotropyEnable    = VK_FALSE;
		samplerCreateInfo.maxAnisotropy       = 1.0;
		samplerCreateInfo.borderColor         = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		mDevice->Validate(vkCreateSampler(mDevice->GetDevice(), &samplerCreateInfo, nullptr, &mSampler));
	}
}

void Texture::CreateImageView(const VkImageCreateInfo& imageCreateInfo)
{
	const VkImageAspectFlagBits aspectMask =
	    mImageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	if (imageCreateInfo.imageType == VK_IMAGE_TYPE_2D)
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	else if (imageCreateInfo.imageType == VK_IMAGE_TYPE_3D)
		imageViewType = VK_IMAGE_VIEW_TYPE_3D;

	if (imageCreateInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else if (mLayers > 1)
		imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	VkImageViewCreateInfo imageViewCreateInfo           = {};
	imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image                           = mImage;
	imageViewCreateInfo.viewType                        = imageViewType;
	imageViewCreateInfo.format                          = mFormat;
	imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
	imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
	imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
	imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
	imageViewCreateInfo.subresourceRange.aspectMask     = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
	imageViewCreateInfo.subresourceRange.levelCount     = mMips;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount     = mLayers;

	mDevice->Validate(vkCreateImageView(mDevice->GetDevice(), &imageViewCreateInfo, nullptr, &mImageView));
}

void Texture::SetImageLayout(VkCommandBuffer cmdBuffer, VkImage image, VkImageLayout oldImageLayout, VkImageLayout newImageLayout,
                             VkImageSubresourceRange subresourceRange)
{
	VkPipelineStageFlags srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkPipelineStageFlags dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

	// Create an image barrier object
	VkImageMemoryBarrier imageMemoryBarrier {};
	imageMemoryBarrier.sType               = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	imageMemoryBarrier.oldLayout        = oldImageLayout;
	imageMemoryBarrier.newLayout        = newImageLayout;
	imageMemoryBarrier.image            = image;
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

	if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED && newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = 0;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
		dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		// throw std::invalid_argument("unsupported layout transition!");
	}

	// Put barrier inside setup command buffer
	vkCmdPipelineBarrier(cmdBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}
