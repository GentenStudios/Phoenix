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

#include <Phoenix/Chunk.hpp>
#include <Phoenix/Phoenix.hpp>
#include <Phoenix/World.hpp>
#include <Phoenix/Mods.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/DeviceMemory.hpp>

#include <Globals/Globals.hpp>
#include <ResourceManager/ResourceManager.hpp>

phx::Chunk::Chunk() { Reset(); }

void phx::Chunk::Reset()
{
	for (uint32_t i = 0; i < MAX_BLOCKS_PER_CHUNK; ++i)
	{
		m_blocks[i] = ModHandler::GetAirBlock();
	}
}

bool phx::Chunk::IsDirty() const { return m_dirty; }

void phx::Chunk::MarkClean() { m_dirty = false; }

void phx::Chunk::SetPosition(const glm::ivec3& position) { m_position = position; }

glm::ivec3 phx::Chunk::GetPosition() const { return m_position; }

phx::ChunkBlock* phx::Chunk::GetBlocks() { return m_blocks; }

void phx::Chunk::SetChunkNeighbours(Neighbours* neighbours) { m_neighbours = neighbours; }

phx::ChunkBlock phx::Chunk::GetBlock(const glm::ivec3& position) const { return m_blocks[GetIndex(position)]; }

void phx::Chunk::SetBlock(const glm::ivec3& position, ChunkBlock block)
{
	m_blocks[GetIndex(position)] = block;
	m_dirty                     = true;
}

phx::Chunk::Neighbours* phx::Chunk::GetNeighbours() const { return m_neighbours; }

// Temp mesh
// clang-format off
const glm::vec3 BLOCK_VERTICES[] = {
    {1.f, 1.f, 1.f}, // east (right)
    {1.f, 1.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 1.f},
	{1.f, 1.f, 1.f},

	{0.f, 0.f, 0.f}, // west
	{0.f, 1.f, 0.f},
    {0.f, 1.f, 1.f}, 
	{0.f, 1.f, 1.f},
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 0.f},

    {1.f, 0.f, 1.f}, // bottom
    {1.f, 0.f, 0.f},
	{0.f, 0.f, 0.f},
	{0.f, 0.f, 0.f},
	{0.f, 0.f, 1.f},
	{1.f, 0.f, 1.f},

    {0.f, 1.f, 0.f}, // top
    {1.f, 1.f, 0.f},
	{1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
	{0.f, 1.f, 1.f},
	{0.f, 1.f, 0.f},
	
	{0.f, 0.f, 0.f},  // north (front)
	{1.f, 0.f, 0.f},
    {1.f, 1.f, 0.f},
	{1.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
    {0.f, 0.f, 0.f},

    {1.f, 1.f, 1.f}, // south
    {1.f, 0.f, 1.f},
	{0.f, 0.f, 1.f}, 
	{0.f, 0.f, 1.f},
	{0.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
};
const glm::vec3 BLOCK_NORMALS[] = {
	

    {1.f, 0.f, 0.f}, // east (right)
    {1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},

	{1.f, 0.f, 0.f}, // west
	{1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f}, 
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f},

    {0.f, 1.f, 0.f}, // bottom
    {0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},

    {0.f, 1.f, 0.f}, // top
    {0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	
	{0.f, 0.f, 1.f},  // north (front)
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},

    {0.f, 0.f, 1.f}, // south
    {0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f}, 
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
};

static const glm::vec2 BLOCK_UVS[] = {

    {1.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},

    {0.f, 0.f},
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
	{0.f, 0.f},

    {1.f, 1.f},
	{1.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{1.f, 1.f},

    {0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
	{1.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},

	{1.f, 1.f},
	{0.f, 1.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{1.f, 0.f},
    {1.f, 1.f},
	
	{1.f, 0.f},
	{1.f, 1.f},
	{0.f, 1.f},
	{0.f, 1.f},
	{0.f, 0.f},
    {1.f, 0.f},
};
// clang-format on
