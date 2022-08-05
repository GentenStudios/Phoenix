#pragma once

#include <vulkan.hpp>

class Allocator
{
public:
	Allocator( );

	void SetMaxAllocationSize( uint32_t size );

	uint32_t Allocate( uint32_t size, uint32_t allignment );

	void ResetAllocation( );
private:
	uint32_t mOffset;
	uint32_t mSize;
};