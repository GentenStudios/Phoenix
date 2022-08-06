#include <Renderer/DeviceMemory.hpp>
#include <Renderer/Device.hpp>

DeviceMemory::DeviceMemory( RenderDevice* device, uint32_t size, uint32_t memoryProperties ) : mDevice( device ), mSize( size )
{

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = size;
	memoryAllocateInfo.memoryTypeIndex = memoryProperties;

	mDevice->Validate( vkAllocateMemory(
		mDevice->GetDevice(),
		&memoryAllocateInfo,
		nullptr,
		&mMemory
	) );

}

DeviceMemory::~DeviceMemory( )
{
	vkFreeMemory(
		mDevice->GetDevice( ),
		mMemory,
		nullptr
	);
}

void DeviceMemory::Map( VkDeviceSize size, VkDeviceSize offset, void*& ptr )
{
	mDevice->Validate( vkMapMemory(
		mDevice->GetDevice( ),
		mMemory,
		offset,
		size,
		0,
		&ptr
	) );
}

void DeviceMemory::UnMap( )
{
	vkUnmapMemory(
		mDevice->GetDevice( ),
		mMemory
	);
}
