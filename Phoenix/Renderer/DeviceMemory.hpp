#pragma once

#include <Renderer/Vulkan.hpp>

class RenderDevice;

class DeviceMemory
{
public:
	DeviceMemory(RenderDevice* device, uint32_t size, uint32_t memoryProperties);
	~DeviceMemory();

	VkDeviceMemory GetMemory() const;
	uint32_t       GetSize() const;

	void Map(VkDeviceSize size, VkDeviceSize offset, void*& ptr);
	void Unmap();

private:
	RenderDevice*  m_device;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	uint32_t       m_size;
};
