#include <Renderer/MemoryAllocator.hpp>

Allocator::Allocator( ) : mOffset(0)
{
}

void Allocator::SetMaxAllocationSize( uint32_t size )
{
	mSize = size;
}

uint32_t Allocator::Allocate( uint32_t size, uint32_t allignment )
{
	uint32_t mod = mOffset % allignment;
	uint32_t modAlignment = allignment - mod;

	if( mod == 0)
		modAlignment = 0;

	if( mOffset + size + modAlignment > mSize )
	{
		return UINT32_MAX;
	}

	mOffset += modAlignment;
	uint32_t start = mOffset;
	mOffset += size;
	return start;
}

void Allocator::ResetAllocation( )
{
	mOffset = 0;
}
