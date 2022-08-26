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

#include <Phoenix/World.hpp>

#include <Phoenix/Chunk.hpp>
#include <Phoenix/Collision.hpp>
#include <Phoenix/Mods.hpp>
#include <Renderer/Buffer.hpp>
#include <Renderer/Camera.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>

#include <ResourceManager/RenderTechnique.hpp>
#include <ResourceManager/ResourceManager.hpp>

const glm::vec3 BLOCK_VERTICES[] = {
    {0.f, 0.f, 0.f}, // north (front)
    {1.f, 0.f, 0.f}, {1.f, 1.f, 0.f}, {1.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 0.f},

	{1.f, 1.f, 1.f}, // east (right)
    {1.f, 1.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 1.f}, {1.f, 1.f, 1.f},

	{1.f, 1.f, 1.f}, // south
    {1.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 1.f, 1.f}, {1.f, 1.f, 1.f},

    {0.f, 0.f, 0.f}, // west
    {0.f, 1.f, 0.f}, {0.f, 1.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 0.f},

    {0.f, 1.f, 0.f}, // top
    {1.f, 1.f, 0.f}, {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, {0.f, 1.f, 1.f}, {0.f, 1.f, 0.f},

	{1.f, 0.f, 1.f}, // bottom
    {1.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 1.f}, {1.f, 0.f, 1.f},

};

const glm::vec3 BLOCK_NORMALS[] = {
    {0.f, 0.f, 1.f}, // north (front)
    {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f},

    {1.f, 0.f, 0.f}, // east (right)
    {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f},

    {1.f, 0.f, 0.f}, // west
    {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f}, {1.f, 0.f, 0.f},

	{0.f, 0.f, 1.f}, // south
    {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f}, {0.f, 0.f, 1.f},

    {0.f, 1.f, 0.f}, // top
    {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f},

    {0.f, 1.f, 0.f}, // bottom
    {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 1.f, 0.f},
};

static const glm::vec2 BLOCK_UVS[] = {

    {1.f, 1.f}, {0.f, 1.f}, {0.f, 0.f}, {0.f, 0.f}, {1.f, 0.f}, {1.f, 1.f},
    {1.f, 0.f}, {0.f, 0.f}, {0.f, 1.f}, {0.f, 1.f}, {1.f, 1.f}, {1.f, 0.f},
    {1.f, 0.f}, {1.f, 1.f}, {0.f, 1.f}, {0.f, 1.f}, {0.f, 0.f}, {1.f, 0.f},
	{0.f, 0.f}, {0.f, 1.f}, {1.f, 1.f}, {1.f, 1.f}, {1.f, 0.f}, {0.f, 0.f},
    {0.f, 1.f}, {1.f, 1.f}, {1.f, 0.f}, {1.f, 0.f}, {0.f, 0.f}, {0.f, 1.f},
    {1.f, 1.f}, {1.f, 0.f}, {0.f, 0.f}, {0.f, 0.f}, {0.f, 1.f}, {1.f, 1.f},
};

phx::WorldData::WorldData(RenderDevice* device, ResourceManager* resourceManager)
    : m_renderDevice(device), m_resourceManager(resourceManager)
{
	m_memoryHeap = m_resourceManager->GetResource<MemoryHeap>("GPUMappableMemoryHeap");

	const uint64_t vertexBufferSize = sizeof(VertexData) * VERTEX_PAGE_SIZE * TOTAL_VERTEX_PAGE_COUNT;
	m_vertexBuffer =
	    std::make_unique<Buffer>(m_renderDevice, m_memoryHeap, vertexBufferSize,
	                             VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE);

	const uint64_t indirectBufferSize = sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT;
	m_indirectDrawsGPU                = std::make_unique<Buffer>(m_renderDevice, m_memoryHeap, indirectBufferSize,
                                                  VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                      VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                  VK_SHARING_MODE_EXCLUSIVE);

	const uint64_t positionBufferSize = sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT;
	m_chunkPositionsGPU               = std::make_unique<Buffer>(m_renderDevice, m_memoryHeap, positionBufferSize,
                                                   VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
                                                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                                   VK_SHARING_MODE_EXCLUSIVE);

	m_freeVertexPageCount = TOTAL_VERTEX_PAGE_COUNT;

	m_indirectDrawsCPU  = std::make_unique<VkDrawIndirectCommand[]>(TOTAL_VERTEX_PAGE_COUNT);
	m_chunkPositionsCPU = std::make_unique<glm::mat4[]>(TOTAL_VERTEX_PAGE_COUNT);

	ResetAllIndirectDraws();
	ResetAllPositions();

	m_indexedIndirectResourceTable =
	    m_resourceManager->GetResource<ResourceTableLayout>("IndexedIndirectCommandResourceTableLayout")->CreateTable();
	m_indexedIndirectResourceTable->Bind(0, m_indirectDrawsGPU.get());

	m_chunkPositionsResourceTable = m_resourceManager->GetResource<ResourceTableLayout>("ChunkPositionResourceTableLayout")->CreateTable();
	m_chunkPositionsResourceTable->Bind(0, m_chunkPositionsGPU.get());

	m_chunksSorted = std::make_unique<ChunkRenderData[]>(MAX_CHUNKS);

	// Setup Vertex Pages.
	m_vertexPages = std::make_unique<VertexMemoryPage[]>(TOTAL_VERTEX_PAGE_COUNT);
	m_freeVertexPages = nullptr;
	for (int i = TOTAL_VERTEX_PAGE_COUNT - 1; i >= 0; --i)
	{
		m_vertexPages[i].index       = i;
		m_vertexPages[i].offset      = VERTEX_PAGE_SIZE * sizeof(ChunkVertexData) * i;
		m_vertexPages[i].vertexCount = 0;
		m_vertexPages[i].next        = m_freeVertexPages;

		m_freeVertexPages = &m_vertexPages[i];
	}
}

phx::WorldData::~WorldData() {}

void phx::WorldData::Update(const glm::ivec3& playerGridPosition, const glm::vec3& playerBoxLocalPosition)
{
	if (m_firstRun)
	{
		// Make sure MAX_WORLD_CHUNKS_PER_AXIS is odd.
		static_assert(MAX_WORLD_CHUNKS_PER_AXIS % 2 == 1);

		const glm::ivec3 currentChunkPos = playerGridPosition / 16;

		// We need to generate the base chunks around the player right now.
		const int iterInEachDir = (MAX_WORLD_CHUNKS_PER_AXIS - 1) / 2;
		uint32_t  index         = 0;
		for (int x = -iterInEachDir; x <= iterInEachDir; ++x)
		{
			for (int y = -iterInEachDir; y <= iterInEachDir; ++y)
			{
				for (int z = -iterInEachDir; z <= iterInEachDir; ++z)
				{
					glm::ivec3 chunkPosition = {x, y, z};
					chunkPosition *= CHUNK_BLOCK_SIZE;
					chunkPosition += currentChunkPos;

					ChunkData* chunk = AddChunk(chunkPosition);

					// Hard-coded dirt.
					constexpr ChunkBlock dirt = {0x00000001};

					// Temporary world generation.
					ChunkBlock blockToSet = dirt;
					if (chunkPosition.y >= 0)
						blockToSet = ModHandler::GetAirBlock();

					auto* blocks = chunk->GetBlocks();
					std::fill_n(blocks, MAX_BLOCKS_PER_CHUNK, blockToSet);

					glm::mat4 renderMat     = glm::translate(glm::mat4 {1.f}, (glm::vec3) chunkPosition);
					m_chunksSorted[index++] = {nullptr, 0, chunkPosition, renderMat, chunk};
				}
			}
		}

		m_firstRun = false;
	}

	// Update View logic
	if (m_lastPlayerGridPosition != playerGridPosition)
	{
		// Do nothing right now.
	}

	m_lastPlayerGridPosition     = playerGridPosition;
	m_lastPlayerBoxLocalPosition = playerBoxLocalPosition;

	for (uint32_t i = 0; i < MAX_CHUNKS; ++i)
	{
		if (m_chunksSorted[i].chunk->IsDirty())
		{
			// Submit for re-mesh.
			RemeshChunk(&m_chunksSorted[i]);
			m_chunksSorted[i].chunk->MarkClean();
		}
	}
}

void phx::WorldData::Draw(VkCommandBuffer* commandBuffer, uint32_t index)
{
	RenderTechnique* standardMaterial = m_resourceManager->GetResource<RenderTechnique>("StandardMaterial");

	standardMaterial->GetPipeline()->Use(commandBuffer, index);

	// todo find a way of auto binding global data for shaders, perhaps a global and local mapping
	m_resourceManager->GetResource<ResourceTable>("CameraResourceTable")
	    ->Use(commandBuffer, index, 0, standardMaterial->GetPipelineLayout()->GetPipelineLayout());
	m_resourceManager->GetResource<ResourceTable>("SamplerArrayResourceTable")
	    ->Use(commandBuffer, index, 1, standardMaterial->GetPipelineLayout()->GetPipelineLayout());

	for (int i = 0; i < TOTAL_VERTEX_PAGE_COUNT; i++)
	{
		VkDeviceSize offsets[] = {sizeof(VertexData) * VERTEX_PAGE_SIZE * i};

		vkCmdBindVertexBuffers(commandBuffer[index], 0, 1, &m_vertexBuffer->GetBuffer(), offsets);

		VkDeviceSize positionOffsets[] = {sizeof(glm::mat4) * i};
		// Position data
		vkCmdBindVertexBuffers(commandBuffer[index], 1, 1, &m_chunkPositionsGPU->GetBuffer(), positionOffsets);
		vkCmdDrawIndirect(commandBuffer[index], m_indirectDrawsGPU->GetBuffer(), sizeof(VkDrawIndirectCommand) * i, 1,
		                  sizeof(VkDrawIndirectCommand));
	}

	RenderTechnique* skybox = m_resourceManager->GetResource<RenderTechnique>("Skybox");

	skybox->GetPipeline()->Use(commandBuffer, index);

	m_resourceManager->GetResource<ResourceTable>("CameraResourceTable")
	    ->Use(commandBuffer, index, 0, skybox->GetPipelineLayout()->GetPipelineLayout());
	m_resourceManager->GetResource<ResourceTable>("SkyboxResourceTable")
	    ->Use(commandBuffer, index, 1, skybox->GetPipelineLayout()->GetPipelineLayout());

	// Vertices are hard baked into the shader.
	vkCmdDraw(commandBuffer[index], 6, 1, 0, 0);
}

phx::ChunkData* phx::WorldData::GetChunkIn(const glm::ivec3& position)
{
	glm::ivec3 steppedPosition = position / static_cast<int>(CHUNK_BLOCK_SIZE);
	steppedPosition *= CHUNK_BLOCK_SIZE;

	const auto it = m_chunks.find(steppedPosition);
	if (it == m_chunks.end())
		return nullptr;

	return &it->second;
}

phx::ChunkBlock phx::WorldData::GetBlock(const glm::ivec3& position)
{
	glm::ivec3 chunkPos = position / static_cast<int>(CHUNK_BLOCK_SIZE);
	glm::ivec3 blockPos = position % static_cast<int>(CHUNK_BLOCK_SIZE);

	const glm::ivec3 chunkModifier = glm::lessThan(blockPos, {0, 0, 0});
	const glm::ivec3 blockModifier = chunkModifier * static_cast<int>(CHUNK_BLOCK_SIZE);

	// Adjust as necessary for negative world space.
	chunkPos -= chunkModifier;
	chunkPos *= CHUNK_BLOCK_SIZE;
	blockPos += blockModifier;

	const auto it = m_chunks.find(chunkPos);
	if (it == m_chunks.end())
		return ModHandler::GetAirBlock();

	return it->second.GetBlock(blockPos);
}

void phx::WorldData::SetBlock(const glm::ivec3& position, ChunkBlock newBlock, Action action)
{
	glm::ivec3 chunkPos = position / static_cast<int>(CHUNK_BLOCK_SIZE);
	glm::ivec3 blockPos = position % static_cast<int>(CHUNK_BLOCK_SIZE);

	const glm::ivec3 chunkModifier = glm::lessThan(blockPos, {0, 0, 0});
	const glm::ivec3 blockModifier = chunkModifier * static_cast<int>(CHUNK_BLOCK_SIZE);

	// Adjust as necessary for negative world space.
	chunkPos -= chunkModifier;
	chunkPos *= CHUNK_BLOCK_SIZE;
	blockPos += blockModifier;

	const auto it = m_chunks.find(chunkPos);
	if (it == m_chunks.end())
		return;

	// Copy of data of old block before setting.
	// We can use this when calling a callback.
	auto oldBlock = it->second.GetBlock(blockPos);

	// Set the new block.
	it->second.SetBlock(blockPos, newBlock);

	switch (action)
	{
	case Action::SET:
		// Nothing.
		break;
	case Action::PLACE:
		printf("Placing block at: %i, %i, %i", blockPos.x, blockPos.y, blockPos.z);
		break;
	case Action::BREAK:
		printf("Breaking block at: %i, %i, %i", blockPos.x, blockPos.y, blockPos.z);
		break;
	}
}

uint32_t phx::WorldData::GetFreeMemoryPoolCount() const { return m_freeVertexPageCount; }

phx::ChunkData* phx::WorldData::AddChunk(const glm::ivec3& position)
{
	const auto it = m_chunks.try_emplace(position);
	if (!it.second)
	{
		return &it.first->second;
	}

	ChunkData* chunk = &it.first->second;

	constexpr glm::ivec3 X_MOD = {CHUNK_BLOCK_SIZE, 0, 0};
	constexpr glm::ivec3 Y_MOD = {0, CHUNK_BLOCK_SIZE, 0};
	constexpr glm::ivec3 Z_MOD = {0, 0, CHUNK_BLOCK_SIZE};

	// Chunk just got added, let's try set neighbours.
	// We're also updating the neighbours of these chunks if they exist.
	// This could potentially end up being quite a slow operation due to the potentially significant indirection.
	const auto north = m_chunks.find(position - Z_MOD);
	const auto east  = m_chunks.find(position + X_MOD);
	const auto south = m_chunks.find(position + Z_MOD);
	const auto west  = m_chunks.find(position - X_MOD);
	const auto above = m_chunks.find(position + Y_MOD);
	const auto below = m_chunks.find(position - Y_MOD);

	ChunkData::Neighbours neighbours;
	if (north != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::NORTH]                     = &north->second;
		north->second.GetNeighbours()->neighbours[ChunkData::SOUTH] = chunk;
	}

	if (east != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::EAST]                    = &east->second;
		east->second.GetNeighbours()->neighbours[ChunkData::WEST] = chunk;
	}

	if (south != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::SOUTH]                     = &south->second;
		south->second.GetNeighbours()->neighbours[ChunkData::NORTH] = chunk;
	}

	if (west != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::WEST]                    = &west->second;
		west->second.GetNeighbours()->neighbours[ChunkData::EAST] = chunk;
	}

	if (above != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::TOP]                        = &above->second;
		above->second.GetNeighbours()->neighbours[ChunkData::BOTTOM] = chunk;
	}

	if (below != m_chunks.end())
	{
		neighbours.neighbours[ChunkData::BOTTOM]                  = &below->second;
		below->second.GetNeighbours()->neighbours[ChunkData::TOP] = chunk;
	}

	ChunkData::Neighbours& ref = m_chunkNeighbours[position];
	ref                        = neighbours;

	chunk->SetChunkNeighbours(&ref);

	return chunk;
}

void phx::WorldData::RemeshChunk(ChunkRenderData* renderData)
{
	renderData->vertexCount = 0;

	void* memoryPtr = nullptr;
	ChunkVertexData* vertexStream = nullptr;

	FreeVertexPages(renderData->vertexPage);

	auto* chunk      = renderData->chunk;
	auto* neighbours = chunk->GetNeighbours()->neighbours;

	for (int x = 0; x < CHUNK_BLOCK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_BLOCK_SIZE; ++y)
		{
			for (int z = 0; z < CHUNK_BLOCK_SIZE; ++z)
			{
				ChunkBlock blockID = renderData->chunk->GetBlock({x, y, z});
				if (blockID == ModHandler::GetAirBlock())
					continue;

				bool visibility[6] = {false};

				const ChunkData* north = neighbours[ChunkData::NORTH];
				const ChunkData* east  = neighbours[ChunkData::EAST];
				const ChunkData* south = neighbours[ChunkData::SOUTH];
				const ChunkData* west  = neighbours[ChunkData::WEST];
				const ChunkData* above = neighbours[ChunkData::TOP];
				const ChunkData* below = neighbours[ChunkData::BOTTOM];

				// North
				if (z == 0)
				{
					if (north != nullptr)
					{
						visibility[ChunkData::NORTH] = north->GetBlock({x, y, CHUNK_BLOCK_SIZE - 1}) == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibility[ChunkData::NORTH] = chunk->GetBlock({x, y, z - 1}) == ModHandler::GetAirBlock();
				}

				// East.
				if (x == CHUNK_BLOCK_SIZE - 1)
				{
					if (east != nullptr)
					{
						visibility[ChunkData::EAST] = east->GetBlock({0, y, z}) == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibility[ChunkData::EAST] = chunk->GetBlock({x + 1, y, z}) == ModHandler::GetAirBlock();
				}

				// South
				if (z == CHUNK_BLOCK_SIZE - 1)
				{
					if (south != nullptr)
					{
						visibility[ChunkData::SOUTH] = south->GetBlock({x, y, 0}) == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibility[ChunkData::SOUTH] = chunk->GetBlock({x, y, z + 1}) == ModHandler::GetAirBlock();
				}

				// West
				if (x == 0)
				{
					if (west != nullptr)
					{
						visibility[ChunkData::WEST] = west->GetBlock({0, y, z}) == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibility[ChunkData::WEST] = chunk->GetBlock({x - 1, y, z}) == ModHandler::GetAirBlock();
				}

				// Above
				if (y == CHUNK_BLOCK_SIZE - 1)
				{
					if (above != nullptr)
						visibility[ChunkData::TOP] = above->GetBlock({x, 0, z}) == ModHandler::GetAirBlock();
				}
				else
				{
					visibility[ChunkData::TOP] = chunk->GetBlock({x, y + 1, z}) == ModHandler::GetAirBlock();
				}

				// Below
				if (y == 0)
				{
					if (below != nullptr)
					{
						visibility[ChunkData::BOTTOM] = below->GetBlock({x, CHUNK_BLOCK_SIZE - 1, z}) == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibility[ChunkData::BOTTOM] = chunk->GetBlock({x, y - 1, z}) == ModHandler::GetAirBlock();
				}

				// For cube models.
				for (int i = 0; i < 6; ++i)
				{
					if (!visibility[i])
						continue;

					if (renderData->vertexPage == nullptr)
					{
						renderData->vertexPage = GetFreeVertexPage();
						if (renderData->vertexPage == nullptr)
						{
							printf("Vertex page congestion. Cannot finish.");

							// Don't do anything.
							continue;
						}

						m_vertexBuffer->GetDeviceMemory()->Map(VERTEX_PAGE_SIZE * sizeof(VertexData),
						                                       m_vertexBuffer->GetMemoryOffset() + renderData->vertexPage->offset,
						                                       memoryPtr);

						vertexStream = static_cast<ChunkVertexData*>(memoryPtr);
					}
					else if (VERTEX_PAGE_SIZE - renderData->vertexPage->vertexCount < 36)
					{
						// Move onto a new page.
						m_vertexBuffer->GetDeviceMemory()->Unmap();

						VertexMemoryPage* newPage = GetFreeVertexPage();
						if (newPage == nullptr)
						{
							printf("Vertex page congestion. Cannot finish.");

							// Don't do anything.
							continue;
						}

						m_vertexBuffer->GetDeviceMemory()->Map(VERTEX_PAGE_SIZE * sizeof(VertexData),
						                                       m_vertexBuffer->GetMemoryOffset() + newPage->offset,
						                                       memoryPtr);

						newPage->next = renderData->vertexPage;
						renderData->vertexPage = newPage;

						vertexStream = static_cast<ChunkVertexData*>(memoryPtr);
					}

					// Temporary.
					auto modHandler = m_resourceManager->GetResource<ModHandler>("ModHandler");

					uint32_t faceTextureID = modHandler->GetBlock(blockID)->textureIndex;

					// Loop through for the face vertices
					for (int j = 0; j < 6; j++)
					{
						const unsigned int lookupIndex = j + (i * 6);

						(*vertexStream).position = BLOCK_VERTICES[lookupIndex];
						(*vertexStream).position.x += x;
						(*vertexStream).position.y += y;
						(*vertexStream).position.z += z;

						(*vertexStream).normal = BLOCK_NORMALS[lookupIndex];

						(*vertexStream).uv = BLOCK_UVS[lookupIndex];

						(*vertexStream).textureID = faceTextureID;

						vertexStream++;
					}

					renderData->vertexPage->vertexCount += 6;
					renderData->vertexCount += 6;
				}
			}
		}	
	}

	if (renderData->vertexPage != nullptr)
		m_vertexBuffer->GetDeviceMemory()->Unmap();

	ProcessVertexPages(renderData->vertexPage, renderData->renderMatrix);
}

void phx::WorldData::ComputeVisibility(VkCommandBuffer* commandBuffer, uint32_t index)
{
	RenderTechnique* frustrumPipeline    = m_resourceManager->GetResource<RenderTechnique>("ViewFrustrumCulling");
	ResourceTable*   cameraResourceTable = m_resourceManager->GetResource<ResourceTable>("CameraResourceTable");

	frustrumPipeline->GetPipeline()->Use(commandBuffer, index);

	cameraResourceTable->Use(commandBuffer, index, 0, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                         VK_PIPELINE_BIND_POINT_COMPUTE);

	m_chunkPositionsResourceTable->Use(commandBuffer, index, 1, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                                   VK_PIPELINE_BIND_POINT_COMPUTE);

	m_indexedIndirectResourceTable->Use(commandBuffer, index, 2, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                                   VK_PIPELINE_BIND_POINT_COMPUTE);

	vkCmdDispatch(commandBuffer[index], TOTAL_VERTEX_PAGE_COUNT, 1, 1);
}

void phx::WorldData::ResetAllIndirectDraws()
{
	constexpr VkDrawIndirectCommand indirectDraw = {0, 0, 0, 0};
	std::fill_n(m_indirectDrawsCPU.get(), TOTAL_VERTEX_PAGE_COUNT, indirectDraw);

	UpdateAllIndirectDraws();
}

void phx::WorldData::ResetAllPositions()
{
	std::fill_n(m_chunkPositionsCPU.get(), TOTAL_VERTEX_PAGE_COUNT, glm::mat4(0.f));

	UpdateAllPositions();
}

void phx::WorldData::UpdateAllIndirectDraws()
{
	m_indirectDrawsGPU->TransferInstantly(m_indirectDrawsCPU.get(), sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT);
}

void phx::WorldData::UpdateAllPositions()
{
	m_chunkPositionsGPU->TransferInstantly(m_chunkPositionsCPU.get(), sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT);
}

phx::VertexMemoryPage* phx::WorldData::GetFreeVertexPage()
{
	VertexMemoryPage* next = m_freeVertexPages;
	if (m_freeVertexPages)
	{
		m_freeVertexPages = m_freeVertexPages->next;
		next->next        = nullptr;
		next->vertexCount = 0;

		m_freeVertexPageCount--;
	}

	return next;
}

void phx::WorldData::ProcessVertexPages(VertexMemoryPage* pages, const glm::mat4& position)
{
	while (pages != nullptr)
	{
		VkDrawIndirectCommand& indirectCommandInstance = m_indirectDrawsCPU[pages->index];
		indirectCommandInstance.vertexCount            = pages->vertexCount;
		indirectCommandInstance.instanceCount          = 1;

		// Transfer the indirect draw request
		m_indirectDrawsGPU->TransferInstantly(&indirectCommandInstance, sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));

		m_chunkPositionsCPU[pages->index] = position;
		m_chunkPositionsGPU->TransferInstantly(&m_chunkPositionsCPU[pages->index], sizeof(glm::mat4),
		                                       sizeof(glm::mat4) * pages->index);

		pages = pages->next;
	}

	UpdateAllPositions();
}

void phx::WorldData::FreeVertexPages(VertexMemoryPage* pages)
{
	VertexMemoryPage* next = nullptr;
	while (pages != nullptr)
	{
		next = pages->next;

		VkDrawIndirectCommand& indirectCommandInstance = m_indirectDrawsCPU.get()[pages->index];
		indirectCommandInstance.vertexCount            = 0;
		indirectCommandInstance.instanceCount          = 0;

		// Transfer the indirect draw request
		m_indirectDrawsGPU->TransferInstantly(&m_indirectDrawsCPU.get()[pages->index], sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));

		pages->next      = m_freeVertexPages;
		m_freeVertexPages = pages;

		m_freeVertexPageCount++;

		pages = next;
	}
}
