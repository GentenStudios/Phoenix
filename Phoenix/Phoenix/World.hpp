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

	class World
	{
	public:
		World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager);

		~World();

		void Update();

		void Draw(VkCommandBuffer* commandBuffer, uint32_t index);

	private:
		void UpdateAllIndirectDraws();

		void UpdateAllPositionBuffers();

		RenderDevice*           mDevice;
		ResourceManager*        mResourceManager;
		std::unique_ptr<Buffer> mVertexBuffer;

		std::unique_ptr<VkDrawIndirectCommand> mIndirectBufferCPU;
		std::unique_ptr<Buffer>                mIndirectDrawCommands;

		std::unique_ptr<glm::mat4> mPositionBufferCPU;
		std::unique_ptr<Buffer>    mPositionBuffer;

		phx::Chunk* mChunks;
	};
} // namespace phx
