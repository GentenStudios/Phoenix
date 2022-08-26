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

#include <Phoenix/Blocks.hpp>

#include <memory>

class Buffer;

namespace phx
{
	struct VertexMemoryPage;

	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		uint32_t  textureID;
	};

	class ModHandler;
	class Chunk;
	class ChunkData;

	struct ChunkNeighbours
	{
		Chunk** neighbouringChunks[6];
	};

	struct ChunkRenderData
	{
		VertexMemoryPage* vertexPage;
		uint32_t          vertexCount;

		glm::vec3 renderPosition;
		glm::mat4 renderMatrix;

		ChunkData* chunk;
	};

	class ChunkData
	{
	public:
		struct Neighbours
		{
			ChunkData* neighbours[6] = {nullptr};
		};

		enum Face
		{
			NORTH,
			EAST,
			SOUTH,
			WEST,
			TOP,
			BOTTOM
		};

	public:
		ChunkData();
		~ChunkData() = default;

		void Reset();

		bool IsDirty() const;
		void MarkClean();

		void       SetPosition(const glm::ivec3& position);
		glm::ivec3 GetPosition() const;

		ChunkBlock* GetBlocks();

		void        SetChunkNeighbours(Neighbours* neighbours);
		Neighbours* GetNeighbours() const;

		ChunkBlock GetBlock(const glm::ivec3& position) const;
		void       SetBlock(const glm::ivec3& position, ChunkBlock block);

		static constexpr uint32_t GetIndex(const glm::ivec3& position)
		{
			return (CHUNK_BLOCK_SIZE * position.z + position.y) * CHUNK_BLOCK_SIZE + position.x;
		}

		static constexpr glm::ivec3 GetPosition(uint32_t index)
		{
			uint32_t x = index % CHUNK_BLOCK_SIZE;
			uint32_t y = (index / CHUNK_BLOCK_SIZE) % CHUNK_BLOCK_SIZE;
			uint32_t z = index / (CHUNK_BLOCK_SIZE * CHUNK_BLOCK_SIZE);

			return {x, y, z};
		}

	private:
		bool m_dirty = true;

		// We use ivec3 since floats will have precision issues as they get larger.
		glm::ivec3 m_position;
		ChunkBlock m_blocks[MAX_BLOCKS_PER_CHUNK];

		Neighbours* m_neighbours;
	};
} // namespace phx
