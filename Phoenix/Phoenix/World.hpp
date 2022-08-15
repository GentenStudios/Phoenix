// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

