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

#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Renderer/Texture.hpp>

Texture::Texture(RenderDevice* device, MemoryHeap* memoryHeap, uint32_t width, uint32_t height, VkFormat format,
                 VkImageUsageFlags imageUsageFlags, char* data)
    : m_device(device), m_memoryHeap(memoryHeap), m_width(width), m_height(height), m_format(format), m_imageUsageFlags(imageUsageFlags)
{
	if (m_memoryHeap == nullptr)
	{
		m_ownMemory = true;
	}

	// Default to 2D texture with 1 layer and 1 mipmap level.
	m_depth  = 1;
	m_layers = 1;
	m_mips   = 1;

	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType         = VK_IMAGE_TYPE_2D;
	imageCreateInfo.extent.width      = m_width;
	imageCreateInfo.extent.height     = m_height;
	imageCreateInfo.extent.depth      = m_depth;
	imageCreateInfo.mipLevels         = m_mips;
	imageCreateInfo.arrayLayers       = m_layers;
	imageCreateInfo.format            = m_format;
	imageCreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.usage             = m_imageUsageFlags | (data != nullptr ? VK_IMAGE_USAGE_TRANSFER_DST_BIT : 0);
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

Texture::Texture(RenderDevice* device, MemoryHeap* memoryHeap, const VkImageCreateInfo& imageCreateInfo, char* data) : m_device(device), m_memoryHeap(memoryHeap)
{
	if (m_memoryHeap == nullptr)
	{
		m_ownMemory = true;
	}

	m_width           = imageCreateInfo.extent.width;
	m_height          = imageCreateInfo.extent.height;
	m_depth           = imageCreateInfo.extent.depth;
	m_layers          = imageCreateInfo.arrayLayers;
	m_mips            = imageCreateInfo.mipLevels;
	m_format          = imageCreateInfo.format;
	m_imageUsageFlags = imageCreateInfo.usage;

	CreateImageFrom(imageCreateInfo);
	TransferData(imageCreateInfo, data);
	CreateSampler(imageCreateInfo);
	CreateImageView(imageCreateInfo);
}

Texture::~Texture()
{
	if (m_ownMemory)
	{
		delete m_memoryHeap;
	}

	if (m_imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		vkDestroySampler(m_device->GetDevice(), m_sampler, nullptr);
	}

	vkDestroyImageView(m_device->GetDevice(), m_imageView, nullptr);
	vkDestroyImage(m_device->GetDevice(), m_image, nullptr);
}

void Texture::CopyBufferRegionsToImage(Buffer* buffer, VkBufferImageCopy* copies, uint32_t count)
{
	VkCommandBuffer copyCmd = m_device->CreateSingleTimeCommand();

	const VkImageAspectFlagBits aspectMask =
	    m_imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask              = aspectMask;
	subresourceRange.baseMipLevel            = 0;
	subresourceRange.levelCount              = m_mips;
	subresourceRange.layerCount              = m_layers;

	SetImageLayout(copyCmd, m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
	vkCmdCopyBufferToImage(copyCmd, buffer->GetBuffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, count, copies);
	SetImageLayout(copyCmd, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
	               (m_imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
	               subresourceRange);

	m_device->EndCommand(copyCmd);
}

void Texture::TransitionImageLayout(VkCommandBuffer& commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	m_device->TransitionImageLayout(commandBuffer, m_image, m_format, oldLayout, newLayout, {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});
}

VkDescriptorImageInfo Texture::GetDescriptorImageInfo()
{
	VkDescriptorImageInfo descriptorImageInfo = {};
	// Provide the descriptor info the texture data it requires
	descriptorImageInfo.sampler     = m_sampler;
	descriptorImageInfo.imageView   = m_imageView;
	descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	return descriptorImageInfo;
}

VkImage Texture::GetImage() const { return m_image; }

VkSampler Texture::GetSampler() const { return m_sampler; }

VkImageView Texture::GetImageView() const { return m_imageView; }

VkFormat Texture::GetFormat() const { return m_format; }

uint32_t Texture::GetWidth() const { return m_width; }

uint32_t Texture::GetHeight() const { return m_height; }

void Texture::CreateImageFrom(const VkImageCreateInfo& imageCreateInfo)
{
	m_device->Validate(vkCreateImage(m_device->GetDevice(), &imageCreateInfo, nullptr, &m_image));

	VkMemoryRequirements imageMemoryRequirements;
	vkGetImageMemoryRequirements(m_device->GetDevice(), m_image, &imageMemoryRequirements);

	if (m_ownMemory)
	{
		m_memoryHeap = new MemoryHeap(m_device, imageMemoryRequirements.size, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	m_imageSize    = static_cast<uint32_t>(imageMemoryRequirements.size);
	m_memoryOffset = m_memoryHeap->Allocate(m_imageSize, imageMemoryRequirements.alignment);

	m_device->Validate(vkBindImageMemory(m_device->GetDevice(), m_image, m_memoryHeap->GetMemory()->GetMemory(), m_memoryOffset));
}

void Texture::TransferData(const VkImageCreateInfo& imageCreateInfo, char* data)
{
	if (data == nullptr)
		return;

	auto transferBuffer = std::make_unique<Buffer>(m_device, m_imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_SHARING_MODE_EXCLUSIVE);

	transferBuffer->TransferInstantly(data, m_imageSize);

	VkCommandBuffer copyCmd = m_device->CreateSingleTimeCommand();

	const VkImageAspectFlagBits aspectMask =
	    m_imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask              = aspectMask;
	subresourceRange.baseMipLevel            = 0;
	subresourceRange.levelCount              = m_mips;
	subresourceRange.layerCount              = m_layers;

	if (data != nullptr)
	{
		VkBufferImageCopy bufferCopyRegion               = {};
		bufferCopyRegion.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel       = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount     = m_layers;
		bufferCopyRegion.imageExtent.width               = m_width;
		bufferCopyRegion.imageExtent.height              = m_height;
		bufferCopyRegion.imageExtent.depth               = m_depth;
		bufferCopyRegion.bufferOffset                    = 0;
		SetImageLayout(copyCmd, m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
		vkCmdCopyBufferToImage(copyCmd, transferBuffer->GetBuffer(), m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);
		SetImageLayout(copyCmd, m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		               (m_imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
		               subresourceRange);
	}
	else
	{

		SetImageLayout(copyCmd, m_image, VK_IMAGE_LAYOUT_UNDEFINED,
		               (m_imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT) ? VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_GENERAL,
		               subresourceRange);
	}

	m_device->EndCommand(copyCmd);
}

void Texture::CreateSampler(const VkImageCreateInfo& imageCreateInfo)
{
	if (m_imageUsageFlags & VK_IMAGE_USAGE_SAMPLED_BIT)
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
		samplerCreateInfo.maxLod              = static_cast<float>(m_mips);
		samplerCreateInfo.anisotropyEnable    = VK_FALSE;
		samplerCreateInfo.maxAnisotropy       = 1.0;
		samplerCreateInfo.borderColor         = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

		m_device->Validate(vkCreateSampler(m_device->GetDevice(), &samplerCreateInfo, nullptr, &m_sampler));
	}
}

void Texture::CreateImageView(const VkImageCreateInfo& imageCreateInfo)
{
	const VkImageAspectFlagBits aspectMask =
	    m_imageUsageFlags & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewType imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	if (imageCreateInfo.imageType == VK_IMAGE_TYPE_2D)
		imageViewType = VK_IMAGE_VIEW_TYPE_2D;
	else if (imageCreateInfo.imageType == VK_IMAGE_TYPE_3D)
		imageViewType = VK_IMAGE_VIEW_TYPE_3D;

	if (imageCreateInfo.flags & VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT)
		imageViewType = VK_IMAGE_VIEW_TYPE_CUBE;
	else if (m_layers > 1)
		imageViewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	VkImageViewCreateInfo imageViewCreateInfo           = {};
	imageViewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.image                           = m_image;
	imageViewCreateInfo.viewType                        = imageViewType;
	imageViewCreateInfo.format                          = m_format;
	imageViewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_R;
	imageViewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_G;
	imageViewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_B;
	imageViewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_A;
	imageViewCreateInfo.subresourceRange.aspectMask     = aspectMask;
	imageViewCreateInfo.subresourceRange.baseMipLevel   = 0;
	imageViewCreateInfo.subresourceRange.levelCount     = m_mips;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount     = m_layers;

	m_device->Validate(vkCreateImageView(m_device->GetDevice(), &imageViewCreateInfo, nullptr, &m_imageView));
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

