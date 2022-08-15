#pragma once

#include <Renderer/Vulkan.hpp>

#include <memory>

class RenderDevice;
class DeviceMemory;
class Buffer;
class MemoryHeap;

class StaticMesh
{
public:
	StaticMesh(RenderDevice* device, MemoryHeap* memoryHeap, char* vertexData, uint32_t vertexDataSize, char* indexData,
	           uint32_t indexDataSize, uint32_t indexCount);

	~StaticMesh() = default;

	void Use(VkCommandBuffer* commandBuffer, uint32_t index) const;
	void Draw(VkCommandBuffer* commandBuffer, uint32_t index) const;

private:
	RenderDevice*           m_device;
	std::unique_ptr<Buffer> m_vertexBuffer;
	std::unique_ptr<Buffer> m_indexBuffer;

	uint32_t m_indexCount;
};
