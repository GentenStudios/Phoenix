#pragma once

#include <Renderer/vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class Buffer;
class MemoryHeap;

class StaticMesh
{
public:
	StaticMesh( RenderDevice* device, MemoryHeap* memoryHeap, char* vertexData, uint32_t vertexDataSize, char* indexData, uint32_t indexDataSize, uint32_t indexCount );
	~StaticMesh( );

	void Use( VkCommandBuffer* commandBuffer, uint32_t index );

	void Draw( VkCommandBuffer* commandBuffer, uint32_t index );
private:

	RenderDevice* mDevice;
	std::unique_ptr<Buffer> mVertexBuffer;
	std::unique_ptr<Buffer> mIndexBuffer;

	uint32_t mIndexCount;
};