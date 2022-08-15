#pragma once

#include <Renderer/MemoryAllocator.hpp>
#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;

class MemoryHeap
{
public:
	MemoryHeap(RenderDevice* device, uint32_t size, VkMemoryPropertyFlags memoryProperties);

	DeviceMemory* GetMemory() const;

	uint32_t Allocate(uint32_t size, uint32_t alignment);

	void ResetAllocation();

private:
	RenderDevice* m_device;
	Allocator     m_allocator;

	std::unique_ptr<DeviceMemory> m_deviceMemory;
	VkMemoryPropertyFlags         m_memoryProperties;
};
