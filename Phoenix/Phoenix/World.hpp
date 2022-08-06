#pragma once

#include <memory>

#include <vulkan.hpp>

class Chunk;
class Buffer;
class RenderDevice;
class MemoryHeap;
class ResourceManager;

class World
{
public:
	World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager);

	~World();

	void Update();

	void Draw(VkCommandBuffer* commandBuffer, uint32_t index);

private:

	void UpdateAllIndirectDraws();

	RenderDevice* mDevice;
	ResourceManager* mResourceManager;
	std::unique_ptr<Buffer> mVertexBuffer;

	std::unique_ptr<VkDrawIndirectCommand> mIndirectBufferCPU;
	std::unique_ptr<Buffer> mIndirectDrawCommands;

	Chunk* mChunks;

};