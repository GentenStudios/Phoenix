#include "memoryheap.hpp"
#include "device.hpp"
#include "devicememory.hpp"

MemoryHeap::MemoryHeap( RenderDevice* device, uint32_t size, VkMemoryPropertyFlags memoryProperties) : mDevice( device ), mMemoryProperties( memoryProperties )
{
	mDeviceMemory = std::unique_ptr<DeviceMemory>( new DeviceMemory( device, size, device->FindMemoryType( memoryProperties ) ) );

	mAllocator.SetMaxAllocationSize( size );
}

uint32_t MemoryHeap::Allocate( uint32_t size, uint32_t allignment )
{
	uint32_t start = mAllocator.Allocate(size, allignment );
	if( start == UINT32_MAX )
	{
		// To do memory defragmentation on no space avaliable
		return 0;
	}
	return start;
}

void MemoryHeap::ResetAllocation( )
{
	mAllocator.ResetAllocation( );
}
