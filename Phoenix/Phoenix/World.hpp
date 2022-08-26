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

#include <Globals/Globals.hpp>
#include <Renderer/Vulkan.hpp>

#include <glm/gtx/hash.hpp>

#include <Phoenix/Chunk.hpp>
#include <memory>
#include <unordered_map>

class Buffer;
class RenderDevice;
class MemoryHeap;
class ResourceManager;
class ResourceTable;

namespace phx
{
	struct ChunkBlock;
	class Chunk;
	class ChunkData;
	struct ChunkRenderData;

	struct ChunkNeighbours;

	struct ChunkVertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		uint32_t  textureID;
	};

	struct VertexMemoryPage
	{
		uint32_t          index;
		uint32_t          offset;
		uint32_t          vertexCount;
		VertexMemoryPage* next = nullptr;
	};

	class WorldData
	{
	public:
		enum class Action
		{
			SET, // This is just a generic "set the block".
			PLACE,
			BREAK
		};

	public:
		WorldData(RenderDevice* device, ResourceManager* resourceManager);
		~WorldData();

		void Update(const glm::ivec3& playerGridPosition, const glm::vec3& playerBoxLocalPosition);
		void ComputeVisibility(VkCommandBuffer* commandBuffer, uint32_t index);

		void Draw(VkCommandBuffer* commandBuffer, uint32_t index);

		ChunkData* GetChunkIn(const glm::ivec3& position);

		ChunkBlock GetBlock(const glm::ivec3& position);
		void       SetBlock(const glm::ivec3& position, ChunkBlock newBlock, Action action = Action::SET);

		uint32_t GetFreeMemoryPoolCount() const;

	private:
		// Doesn't do anything if already exists.
		// Must be an already stepped chunk position.
		ChunkData* AddChunk(const glm::ivec3& position);

		void RemeshChunk(ChunkRenderData* renderData);

		void ResetAllIndirectDraws();
		void ResetAllPositions();

		void UpdateAllIndirectDraws();
		void UpdateAllPositions();

		VertexMemoryPage* GetFreeVertexPage();

		void ProcessVertexPages(VertexMemoryPage* pages, const glm::mat4& position);
		void FreeVertexPages(VertexMemoryPage* pages);

	private:
		RenderDevice*    m_renderDevice;
		ResourceManager* m_resourceManager;
		MemoryHeap*      m_memoryHeap;

		bool       m_firstRun                   = true;
		glm::ivec3 m_lastPlayerGridPosition     = {0, 0, 0};
		glm::vec3  m_lastPlayerBoxLocalPosition = {0, 0, 0};

		std::unique_ptr<Buffer>             m_vertexBuffer;
		std::unique_ptr<VertexMemoryPage[]> m_vertexPages;
		uint32_t                            m_freeVertexPageCount = 0;

		VertexMemoryPage* m_freeVertexPages = nullptr;

		ResourceTable*                           m_indexedIndirectResourceTable;
		std::unique_ptr<VkDrawIndirectCommand[]> m_indirectDrawsCPU;
		std::unique_ptr<Buffer>                  m_indirectDrawsGPU;

		ResourceTable*               m_chunkPositionsResourceTable;
		std::unique_ptr<glm::mat4[]> m_chunkPositionsCPU;
		std::unique_ptr<Buffer>      m_chunkPositionsGPU;

		// This is the "map" of chunks we're using for now - not ideal but this is much easier to deal with for now.
		// Todo: replace with something else that could potentially perform better on iterations.
		std::unordered_map<glm::ivec3, ChunkData>             m_chunks;
		std::unordered_map<glm::ivec3, ChunkData::Neighbours> m_chunkNeighbours;

		// This is the sorted chunks in the current view.
		std::unique_ptr<ChunkRenderData[]> m_chunksSorted;
	};
} // namespace phx
