#pragma once

#include <Renderer/Vulkan.hpp>

class Allocator
{
public:
	Allocator() = default;

	uint32_t Allocate(uint32_t size, uint32_t alignment);

	void SetMaxAllocationSize(uint32_t size);
	void ResetAllocation();

private:
	uint32_t m_offset = 0;
	uint32_t m_size = 0;
};
