#pragma once

#include <vulkan.hpp>

#include <memory>

#include "memoryallocator.hpp"

class RenderDevice;
class DeviceMemory;
 
class MemoryHeap
{
public:
	MemoryHeap( RenderDevice* device, uint32_t size, VkMemoryPropertyFlags memoryProperties );

	DeviceMemory* GetMemory( ) { return mDeviceMemory.get( ); }

	uint32_t Allocate( uint32_t size, uint32_t allignment );

	void ResetAllocation( );

private:
	RenderDevice* mDevice;

	std::unique_ptr<DeviceMemory> mDeviceMemory;

	Allocator mAllocator;

	VkMemoryPropertyFlags mMemoryProperties;
};