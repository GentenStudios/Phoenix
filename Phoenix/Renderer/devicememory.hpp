#pragma once

#include <Renderer/vulkan.hpp>

class RenderDevice;
class DeviceMemory
{
public:
	DeviceMemory( RenderDevice* device, uint32_t size, uint32_t memoryProperties );
	~DeviceMemory( );

	VkDeviceMemory GetMemory( ) { return mMemory; }

	void Map( VkDeviceSize size, VkDeviceSize offset, void*& ptr );

	void UnMap( );

	uint32_t GetSize(){ return mSize; }
private:
	RenderDevice* mDevice;
	VkDeviceMemory mMemory;
	uint32_t mSize;
};