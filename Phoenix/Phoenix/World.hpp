#pragma once

#include <memory>

#include <Globals/Globals.hpp>

#include <Renderer/Vulkan.hpp>

class Buffer;
class RenderDevice;
class MemoryHeap;
class ResourceManager;

namespace phx
{
	class Chunk;
	class ChunkNabours;

	struct VertexPage
	{
		uint32_t index;
		uint32_t offset;
		uint32_t vertexCount;
		VertexPage* next;
	};

	class World
	{
		friend class Chunk;
	public:
		World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager);

		~World();

		void Update();

		void Draw(VkCommandBuffer* commandBuffer, uint32_t index);

		VertexPage* GetFreeVertexPage();

		unsigned int GetFreeMemoryPoolCount();

	private:
		void UpdateAllIndirectDraws();

		void UpdateAllPositionBuffers();

		void ProcessVertexPages(VertexPage* pages, glm::mat4 position);

		void FreeVertexPages(VertexPage* pages);

		RenderDevice*           mDevice;
		ResourceManager*        mResourceManager;
		std::unique_ptr<Buffer> mVertexBuffer;

		std::unique_ptr<VertexPage> mVertexPages;

		VertexPage* mFreeVertexPages;

		std::unique_ptr<VkDrawIndirectCommand> mIndirectBufferCPU;
		std::unique_ptr<Buffer>                mIndirectDrawCommands;

		std::unique_ptr<glm::mat4> mPositionBufferCPU;
		std::unique_ptr<Buffer>    mPositionBuffer;

		unsigned int mFreeMemoryPoolCount;

		ChunkNabours* mChunkNeighbours;

		// All chunks sorted in grid alignment
		phx::Chunk** mChunksSorted;

		// All chunks as they are allocated in memory
		phx::Chunk* mChunks;
	};
} // namespace phx
