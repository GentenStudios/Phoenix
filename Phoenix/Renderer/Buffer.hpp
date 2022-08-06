#pragma once

#include <Renderer/Vulkan.hpp>
#include <Renderer/MemoryHeap.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;

class Buffer
{
public:
	Buffer( RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharing_mode );
	Buffer( RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharing_mode );
	~Buffer( );

	VkBuffer& GetBuffer( ) { return mBuffer; }
	uint32_t GetMemoryOffset( ) { return mMemoryOffset; }
	uint32_t GetAllocationSize( ) { return mAllocationSize; }
	uint32_t GetBufferSize( ) { return mBufferSize; }
	MemoryHeap* GetMemoryHeap( ) { return mMemoryHeap; }
	DeviceMemory* GetDeviceMemory( ) { return mDeviceMemory; }

	VkDescriptorBufferInfo GetDescriptorInfo( );

	void TransferInstantly(void* ptr, uint32_t size, uint32_t offset = 0);

private:

	VkBuffer CreateStaging( );

	VkBuffer CreateStaging( unsigned int bufferSize );

	RenderDevice* mDevice;
	VkBuffer mBuffer;
	uint32_t mMemoryOffset;
	uint32_t mAllocationSize;
	uint32_t mBufferSize;
	MemoryHeap* mMemoryHeap = nullptr;
	DeviceMemory* mDeviceMemory = nullptr;
};