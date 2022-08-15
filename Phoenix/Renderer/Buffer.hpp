#pragma once

#include <Renderer/MemoryHeap.hpp>
#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;

class Buffer
{
public:
	Buffer(RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);
	Buffer(RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode);
	~Buffer();

	VkBuffer&     GetBuffer();
	uint32_t      GetMemoryOffset() const;
	uint32_t      GetAllocationSize() const;
	uint32_t      GetBufferSize() const;
	MemoryHeap*   GetMemoryHeap() const;
	DeviceMemory* GetDeviceMemory() const;

	VkDescriptorBufferInfo GetDescriptorInfo() const;

	void TransferInstantly(void* ptr, uint32_t size, uint32_t offset = 0) const;

private:
	VkBuffer CreateStaging() const;
	VkBuffer CreateStaging(unsigned int bufferSize) const;

private:
	RenderDevice* m_device;

	MemoryHeap*   m_memoryHeap   = nullptr;
	DeviceMemory* m_deviceMemory = nullptr;
	uint32_t      m_memoryOffset;
	uint32_t      m_allocationSize;

	VkBuffer m_buffer = VK_NULL_HANDLE;
	uint32_t m_bufferSize;
};
