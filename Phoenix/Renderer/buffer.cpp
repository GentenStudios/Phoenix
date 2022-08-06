
#include <Renderer/buffer.hpp>
#include <Renderer/device.hpp>
#include <Renderer/devicememory.hpp>
#include <Renderer/memoryheap.hpp>

#include <assert.h>
#include <string.h>

Buffer::Buffer( RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode ) :
	mDevice( device ), mBufferSize( static_cast<uint32_t>(size) ), mMemoryHeap( memoryHeap ), mDeviceMemory( VK_NULL_HANDLE )
{
	mBuffer = VK_NULL_HANDLE;

	mDeviceMemory = mMemoryHeap->GetMemory( );

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = mBufferSize;
	bufferCreateInfo.usage = usage;
	bufferCreateInfo.sharingMode = sharingMode;

	mDevice->Validate( vkCreateBuffer(
		mDevice->GetDevice(),
		&bufferCreateInfo,
		nullptr,
		&mBuffer
	) );

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(
		mDevice->GetDevice( ),
		mBuffer,
		&bufferMemoryRequirements
	);

	mAllocationSize = (uint32_t) bufferMemoryRequirements.size;

	mMemoryOffset = mMemoryHeap->Allocate(mAllocationSize, static_cast<uint32_t>( bufferMemoryRequirements.alignment ) );

	mDevice->Validate( vkBindBufferMemory(
		mDevice->GetDevice( ),
		mBuffer,
		mMemoryHeap->GetMemory()->GetMemory(),
		mMemoryOffset
	) );
}

Buffer::Buffer( RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode ) :
	mDevice( device ), mBufferSize( static_cast<uint32_t>(size) ), mMemoryHeap(nullptr)
{
	mBuffer = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = mBufferSize;
	bufferCreateInfo.usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferCreateInfo.sharingMode = sharingMode;

	mDevice->Validate( vkCreateBuffer(
		mDevice->GetDevice( ),
		&bufferCreateInfo,
		nullptr,
		&mBuffer
	) );

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(
		mDevice->GetDevice( ),
		mBuffer,
		&bufferMemoryRequirements
	);

	mDeviceMemory = new DeviceMemory( device, static_cast<uint32_t>( bufferMemoryRequirements.size ), device->FindMemoryType( VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT ) );

	mMemoryOffset = 0;

	mAllocationSize = (uint32_t) bufferMemoryRequirements.size;

	mDevice->Validate( vkBindBufferMemory(
		mDevice->GetDevice( ),
		mBuffer,
		mDeviceMemory->GetMemory( ),
		mMemoryOffset
	) );
}

Buffer::~Buffer( )
{
	vkDestroyBuffer(
		mDevice->GetDevice( ),
		mBuffer,
		nullptr
	);
	if ( mMemoryHeap == nullptr )
	{
		delete mDeviceMemory;
	}
}

VkDescriptorBufferInfo Buffer::GetDescriptorInfo( )
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer = mBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = mBufferSize;
	return bufferInfo;
}

void Buffer::TransferInstantly( void* ptr, uint32_t size, uint32_t offset)
{
	void* memoryPtr = nullptr;
	GetDeviceMemory()->Map( size, mMemoryOffset + offset, memoryPtr );
	memcpy( memoryPtr, ptr, size);
	GetDeviceMemory( )->UnMap( );
}

VkBuffer Buffer::CreateStaging( )
{
	VkBuffer staging = VK_NULL_HANDLE;
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = mBufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	mDevice->Validate( vkCreateBuffer(
		mDevice->GetDevice( ),
		&bufferCreateInfo,
		nullptr,
		&staging
	) );
	return staging;
}


VkBuffer Buffer::CreateStaging( unsigned int bufferSize )
{
	VkBuffer staging = VK_NULL_HANDLE;
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = bufferSize;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	mDevice->Validate( vkCreateBuffer(
		mDevice->GetDevice( ),
		&bufferCreateInfo,
		nullptr,
		&staging
	) );
	return staging;
}
