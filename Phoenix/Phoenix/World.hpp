#pragma once

#include <memory>

#include <Globals/Globals.hpp>

#include <Renderer/Vulkan.hpp>

class Buffer;
class RenderDevice;
class MemoryHeap;
class ResourceManager;
class ResourceTable;

namespace phx
{
	class Chunk;
	struct ChunkNeighbours;

	struct VertexPage
	{
		uint32_t    index;
		uint32_t    offset;
		uint32_t    vertexCount;
		VertexPage* next;
	};

	class World
	{
		friend class Chunk;

	public:
		World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager);

		~World();

		void Update();

		void ComputeVisibility(VkCommandBuffer* commandBuffer, uint32_t index);

		void Draw(VkCommandBuffer* commandBuffer, uint32_t index);

		VertexPage* GetFreeVertexPage();

		unsigned int GetFreeMemoryPoolCount();

		void DestroyBlockFromView();

		void PlaceBlockFromView();

	private:
		enum RaycastMode
		{
			Place,
			Destroy
		};

		void RaycastToBlock(float step, int iteration, Chunk*& chunk, int& localX, int& localY, int& localZ, RaycastMode mode);

		void UpdateAllIndirectDraws();

		void UpdateAllPositionBuffers();

		void ProcessVertexPages(VertexPage* pages, glm::mat4 position);

		void FreeVertexPages(VertexPage* pages);

		RenderDevice*           mDevice;
		ResourceManager*        mResourceManager;
		std::unique_ptr<Buffer> mVertexBuffer;

		std::unique_ptr<VertexPage> mVertexPages;

		VertexPage* mFreeVertexPages;

		ResourceTable*                         mIndexedIndirectResourceTable;
		std::unique_ptr<VkDrawIndirectCommand> mIndirectBufferCPU;
		std::unique_ptr<Buffer>                mIndirectDrawCommands;

		ResourceTable*             mChunkPositionsResourceTable;
		std::unique_ptr<glm::mat4> mPositionBufferCPU;
		std::unique_ptr<Buffer>    mPositionBuffer;

		unsigned int mFreeMemoryPoolCount;

		ChunkNeighbours* mChunkNeighbours;

		// All chunks sorted in grid alignment
		phx::Chunk** mChunksSorted;

		// All chunks as they are allocated in memory
		phx::Chunk* mChunks;
	};
} // namespace phx
