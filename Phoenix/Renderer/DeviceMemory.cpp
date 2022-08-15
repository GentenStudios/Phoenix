#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>

DeviceMemory::DeviceMemory(RenderDevice* device, uint32_t size, uint32_t memoryProperties) : m_device(device), m_size(size)
{

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize       = size;
	memoryAllocateInfo.memoryTypeIndex      = memoryProperties;

	m_device->Validate(vkAllocateMemory(m_device->GetDevice(), &memoryAllocateInfo, nullptr, &m_memory));
}

DeviceMemory::~DeviceMemory() { vkFreeMemory(m_device->GetDevice(), m_memory, nullptr); }

VkDeviceMemory DeviceMemory::GetMemory() const { return m_memory; }
uint32_t       DeviceMemory::GetSize() const { return m_size; }

void DeviceMemory::Map(VkDeviceSize size, VkDeviceSize offset, void*& ptr)
{
	m_device->Validate(vkMapMemory(m_device->GetDevice(), m_memory, offset, size, 0, &ptr));
}

void DeviceMemory::Unmap() { vkUnmapMemory(m_device->GetDevice(), m_memory); }
