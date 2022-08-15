#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>

MemoryHeap::MemoryHeap(RenderDevice* device, uint32_t size, VkMemoryPropertyFlags memoryProperties)
    : m_device(device), m_memoryProperties(memoryProperties)
{
	m_deviceMemory = std::make_unique<DeviceMemory>(device, size, device->FindMemoryType(memoryProperties));
	m_allocator.SetMaxAllocationSize(size);
}

DeviceMemory* MemoryHeap::GetMemory() const { return m_deviceMemory.get(); }

uint32_t MemoryHeap::Allocate(uint32_t size, uint32_t alignment)
{
	uint32_t start = m_allocator.Allocate(size, alignment);
	if (start == UINT32_MAX)
	{
		// To do memory defragmentation on no space available
		return 0;
	}

	return start;
}

void MemoryHeap::ResetAllocation() { m_allocator.ResetAllocation(); }
