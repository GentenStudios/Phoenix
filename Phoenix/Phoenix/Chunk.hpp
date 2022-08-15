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
	class World;
	struct VertexPage;
	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		uint32_t textureID;
	};

	class ModHandler;
	class Chunk;

	struct ChunkNeighbours
	{
		Chunk** neighbouringChunks[6];
	};

	class Chunk
	{

	public:

		enum Face
		{
			East,
			West,
			Top,
			Bottom,
			North,
			South,
		};

		Chunk();
		~Chunk() = default;

		void Initialize(World* world, Buffer* vertexBuffer, ModHandler* modHandler);

		void SetPosition(glm::ivec3 position);

		void SetRenderPosition(glm::vec3 position);

		void Reset();

		void GenerateWorld();

		void Update();

		unsigned int GetTotalVertexCount();

		void SetNeighbouringChunk(ChunkNeighbours* neighbouringChunks);

		ChunkBlock GetBlock(int x, int y, int z);
		void     SetBlock(int x, int y, int z, ChunkBlock block);

		void MarkDirty();

		glm::ivec3 GetPosition();

		ChunkNeighbours* GetNabours();


	private:
		void GenerateMesh();

	private:
		World*       m_world;
		unsigned int m_totalVertexCount = 0;
		Buffer*      m_vertexBuffer       = nullptr;

		ModHandler* m_modHandler;

		ChunkNeighbours* m_neighbouringChunk;

		VertexPage* m_vertexPage;

		bool m_dirty = true;

		glm::ivec3 m_position;
		glm::mat4 m_matrix;

		ChunkBlock m_blocks[CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE];
	};
} // namespace phx

