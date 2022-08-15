
#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>

#include <cassert>
#include <cstring>

Buffer::Buffer(RenderDevice* device, MemoryHeap* memoryHeap, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
    : m_device(device), m_memoryHeap(memoryHeap), m_bufferSize(static_cast<uint32_t>(size))
{
	m_deviceMemory = m_memoryHeap->GetMemory();

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = usage;
	bufferCreateInfo.sharingMode        = sharingMode;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &m_buffer));

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &bufferMemoryRequirements);

	m_allocationSize = static_cast<uint32_t>(bufferMemoryRequirements.size);
	m_memoryOffset   = m_memoryHeap->Allocate(m_allocationSize, static_cast<uint32_t>(bufferMemoryRequirements.alignment));

	m_device->Validate(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_memoryHeap->GetMemory()->GetMemory(), m_memoryOffset));
}

Buffer::Buffer(RenderDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkSharingMode sharingMode)
    : m_device(device), m_bufferSize(static_cast<uint32_t>(size))
{
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	bufferCreateInfo.sharingMode        = sharingMode;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &m_buffer));

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(m_device->GetDevice(), m_buffer, &bufferMemoryRequirements);

	m_deviceMemory = new DeviceMemory(device, static_cast<uint32_t>(bufferMemoryRequirements.size),
	                                 device->FindMemoryType(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

	m_allocationSize = static_cast<uint32_t>(bufferMemoryRequirements.size);
	m_memoryOffset   = 0;

	m_device->Validate(vkBindBufferMemory(m_device->GetDevice(), m_buffer, m_deviceMemory->GetMemory(), m_memoryOffset));
}

Buffer::~Buffer()
{
	vkDestroyBuffer(m_device->GetDevice(), m_buffer, nullptr);
	if (m_memoryHeap == nullptr)
	{
		delete m_deviceMemory;
	}
}

VkBuffer& Buffer::GetBuffer() { return m_buffer; }

uint32_t Buffer::GetMemoryOffset() const { return m_memoryOffset; }

uint32_t Buffer::GetAllocationSize() const { return m_allocationSize; }

uint32_t Buffer::GetBufferSize() const { return m_bufferSize; }

MemoryHeap* Buffer::GetMemoryHeap() const { return m_memoryHeap; }

DeviceMemory* Buffer::GetDeviceMemory() const { return m_deviceMemory; }

VkDescriptorBufferInfo Buffer::GetDescriptorInfo() const
{
	VkDescriptorBufferInfo bufferInfo = {};
	bufferInfo.buffer                 = m_buffer;
	bufferInfo.offset                 = 0;
	bufferInfo.range                  = m_bufferSize;

	return bufferInfo;
}

void Buffer::TransferInstantly(void* ptr, uint32_t size, uint32_t offset) const
{
	void* memoryPtr = nullptr;
	GetDeviceMemory()->Map(size, m_memoryOffset + offset, memoryPtr);
	memcpy(memoryPtr, ptr, size);
	GetDeviceMemory()->UnMap();
}

VkBuffer Buffer::CreateStaging() const
{
	VkBuffer staging = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = m_bufferSize;
	bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &staging));
	return staging;
}

VkBuffer Buffer::CreateStaging(unsigned int bufferSize) const
{
	VkBuffer staging = VK_NULL_HANDLE;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size               = bufferSize;
	bufferCreateInfo.usage              = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;

	m_device->Validate(vkCreateBuffer(m_device->GetDevice(), &bufferCreateInfo, nullptr, &staging));
	return staging;
}
