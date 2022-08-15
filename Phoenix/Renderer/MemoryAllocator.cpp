#include <Renderer/MemoryAllocator.hpp>

uint32_t Allocator::Allocate(uint32_t size, uint32_t alignment)
{
	const uint32_t mod          = m_offset % alignment;
	uint32_t       modAlignment = alignment - mod;

	if (mod == 0)
		modAlignment = 0;

	if (m_offset + size + modAlignment > m_size)
	{
		return UINT32_MAX;
	}

	m_offset += modAlignment;
	const uint32_t start = m_offset;
	m_offset += size;
	return start;
}

void Allocator::SetMaxAllocationSize(uint32_t size) { m_size = size; }

void Allocator::ResetAllocation() { m_offset = 0; }
