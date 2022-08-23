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

phx::WorldData::WorldData(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager)
    : m_renderDevice(device), m_resourceManager(resourceManager), m_memoryHeap(memoryHeap)
{
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

	m_chunksSorted = std::make_unique<ChunkData*[]>(MAX_CHUNKS);
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
		if (m_chunksSorted[i]->IsDirty())
		{
			// Submit for re-mesh.
		}
	}
}

void phx::WorldData::Draw(VkCommandBuffer* commandBuffer, uint32_t index) {}

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
	auto currentBlock = it->second.GetBlock(blockPos);

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

uint32_t phx::WorldData::GetFreeMemoryPoolCount() { return m_freeVertexPageCount; }

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

void phx::WorldData::ComputeVisibility(VkCommandBuffer* commandBuffer, uint32_t index) {}

void phx::WorldData::ResetAllIndirectDraws()
{
	constexpr VkDrawIndirectCommand indirectDraw = {0, 0, 0, 0};
	std::fill_n(m_indirectDrawsCPU.get(), TOTAL_VERTEX_PAGE_COUNT, indirectDraw);

	UpdateAllIndirectDraws();
}

void phx::WorldData::ResetAllPositions()
{
	std::fill_n(m_chunkPositionsCPU.get(), TOTAL_VERTEX_PAGE_COUNT, glm::mat4(1.f));

	UpdateAllPositions();
}

void phx::WorldData::UpdateAllIndirectDraws()
{
	m_indirectDrawsGPU->TransferInstantly(m_indirectDrawsCPU.get(), sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT);
}

void phx::WorldData::UpdateAllPositions()
{
	m_chunkPositionsGPU->TransferInstantly(m_chunkPositionsGPU.get(), sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT);
}

phx::VertexPage* phx::WorldData::GetFreeVertexPage() { return nullptr; }

void phx::WorldData::ProcessVertexPages(VertexPage* pages, const glm::mat4& position) {}

void phx::WorldData::FreeVertexPages(VertexPage* page) {}

phx::World::World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager)
    : mDevice(device), mResourceManager(resourceManager)
{
	mVertexBuffer = std::unique_ptr<Buffer>(
	    new Buffer(mDevice, memoryHeap, VERTEX_PAGE_SIZE * sizeof(VertexData) * TOTAL_VERTEX_PAGE_COUNT,
	               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	               VK_SHARING_MODE_EXCLUSIVE));

	mIndirectDrawCommands = std::unique_ptr<Buffer>(new Buffer(mDevice, memoryHeap, sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT,
	                                                           VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
	                                                               VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                                                           VK_SHARING_MODE_EXCLUSIVE));

	mPositionBuffer = std::unique_ptr<Buffer>(new Buffer(mDevice, memoryHeap, sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT,
	                                                     VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
	                                                         VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                                                     VK_SHARING_MODE_EXCLUSIVE));

	mFreeMemoryPoolCount = TOTAL_VERTEX_PAGE_COUNT;

	mIndirectBufferCPU = std::unique_ptr<VkDrawIndirectCommand>(new VkDrawIndirectCommand[TOTAL_VERTEX_PAGE_COUNT]);

	VkDrawIndirectCommand indirectCommandInstance {};
	indirectCommandInstance.vertexCount   = 0;
	indirectCommandInstance.instanceCount = 0;
	indirectCommandInstance.firstInstance = 0;
	indirectCommandInstance.firstVertex   = 0;

	for (uint32_t i = 0; i < TOTAL_VERTEX_PAGE_COUNT; i++)
	{
		mIndirectBufferCPU.get()[i] = indirectCommandInstance;
	}

	mIndexedIndirectResourceTable =
	    mResourceManager->GetResource<ResourceTableLayout>("IndexedIndirectCommandResourceTableLayout")->CreateTable();
	mIndexedIndirectResourceTable->Bind(0, mIndirectDrawCommands.get());

	mChunkPositionsResourceTable = mResourceManager->GetResource<ResourceTableLayout>("ChunkPositionResourceTableLayout")->CreateTable();
	mChunkPositionsResourceTable->Bind(0, mPositionBuffer.get());

	UpdateAllIndirectDraws();

	mChunks          = new Chunk[MAX_CHUNKS];
	mChunksSorted    = new Chunk*[MAX_CHUNKS];
	mChunkNeighbours = new ChunkNeighbours[MAX_CHUNKS];

	mFreeVertexPages = nullptr;

	mPositionBufferCPU = std::unique_ptr<glm::mat4>(new glm::mat4[TOTAL_VERTEX_PAGE_COUNT]);
	mVertexPages       = std::unique_ptr<VertexPage>(new VertexPage[TOTAL_VERTEX_PAGE_COUNT]);

	for (int i = TOTAL_VERTEX_PAGE_COUNT - 1; i >= 0; --i)
	{
		mPositionBufferCPU.get()[i] = glm::mat4(1.0f);

		mVertexPages.get()[i].index       = i;
		mVertexPages.get()[i].offset      = (VERTEX_PAGE_SIZE * sizeof(VertexData)) * i;
		mVertexPages.get()[i].vertexCount = 0;
		mVertexPages.get()[i].next        = mFreeVertexPages;

		mFreeVertexPages = &mVertexPages.get()[i];
	}

	for (int z = 0; z < MAX_WORLD_CHUNKS_PER_AXIS; ++z)
	{
		for (int y = 0; y < MAX_WORLD_CHUNKS_PER_AXIS; ++y)
		{
			for (int x = 0; x < MAX_WORLD_CHUNKS_PER_AXIS; ++x)
			{
				int index = x + (y * MAX_WORLD_CHUNKS_PER_AXIS) + (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS);

				mChunks[index].SetPosition(glm::ivec3(x, y, z));
				mChunks[index].SetRenderPosition(glm::vec3(x, y, z));

				// On start up the chunks are already sorted
				mChunksSorted[index] = &mChunks[index];
			}
		}
	}

	for (int z = 0; z < MAX_WORLD_CHUNKS_PER_AXIS; ++z)
	{
		for (int y = 0; y < MAX_WORLD_CHUNKS_PER_AXIS; ++y)
		{
			for (int x = 0; x < MAX_WORLD_CHUNKS_PER_AXIS; ++x)
			{
				int index = x + (y * MAX_WORLD_CHUNKS_PER_AXIS) + (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS);

				ChunkNeighbours* chunkNabours = &mChunkNeighbours[index];

				chunkNabours->neighbouringChunks[Chunk::East] =
				    (x - 1) < 0 ? nullptr
				                : &mChunksSorted[(x - 1) + (y * MAX_WORLD_CHUNKS_PER_AXIS) +
				                                 (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				chunkNabours->neighbouringChunks[Chunk::West] =
				    (x + 1) >= MAX_WORLD_CHUNKS_PER_AXIS ? nullptr
				                                         : &mChunksSorted[(x + 1) + (y * MAX_WORLD_CHUNKS_PER_AXIS) +
				                                                          (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				chunkNabours->neighbouringChunks[Chunk::Top] =
				    (y + 1) >= MAX_WORLD_CHUNKS_PER_AXIS ? nullptr
				                                         : &mChunksSorted[x + ((y + 1) * MAX_WORLD_CHUNKS_PER_AXIS) +
				                                                          (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				chunkNabours->neighbouringChunks[Chunk::Bottom] =
				    (y - 1) < 0 ? nullptr
				                : &mChunksSorted[x + ((y - 1) * MAX_WORLD_CHUNKS_PER_AXIS) +
				                                 (z * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				chunkNabours->neighbouringChunks[Chunk::South] =
				    (z - 1) < 0 ? nullptr
				                : &mChunksSorted[x + (y * MAX_WORLD_CHUNKS_PER_AXIS) +
				                                 ((z - 1) * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				chunkNabours->neighbouringChunks[Chunk::North] =
				    (z + 1) >= MAX_WORLD_CHUNKS_PER_AXIS
				        ? nullptr
				        : &mChunksSorted[x + (y * MAX_WORLD_CHUNKS_PER_AXIS) +
				                         ((z + 1) * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS)];

				mChunksSorted[index]->SetNeighbouringChunk(chunkNabours);
			}
		}
	}

	ModHandler* modHandler = resourceManager->GetResource<ModHandler>("ModHandler");
	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].Initialize(this, mVertexBuffer.get(), modHandler);

		int x = (MAX_WORLD_CHUNKS_PER_AXIS / 2);
		int y = (MAX_WORLD_CHUNKS_PER_AXIS / 2);
		int z = (MAX_WORLD_CHUNKS_PER_AXIS / 2);

		x -= i % MAX_WORLD_CHUNKS_PER_AXIS;
		y -= (i / MAX_WORLD_CHUNKS_PER_AXIS) % MAX_WORLD_CHUNKS_PER_AXIS;
		z -= i / (MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS);

		x *= CHUNK_BLOCK_SIZE;
		y *= CHUNK_BLOCK_SIZE;
		z *= CHUNK_BLOCK_SIZE;

		mChunks[i].SetPosition(glm::ivec3(x, y, z));
		mChunks[i].SetRenderPosition(glm::vec3(x, y, z));
	}

	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].GenerateWorld();
	}

	UpdateAllPositionBuffers();
}

phx::World::~World()
{
	mVertexBuffer.reset();

	delete[] mChunks;
	delete[] mChunksSorted;
}

void phx::World::Update()
{
	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].Update();
	}
}

void phx::World::ComputeVisibility(VkCommandBuffer* commandBuffer, uint32_t index)
{

	RenderTechnique* frustrumPipeline    = mResourceManager->GetResource<RenderTechnique>("ViewFrustrumCulling");
	ResourceTable*   cameraResourceTable = mResourceManager->GetResource<ResourceTable>("CameraResourceTable");

	frustrumPipeline->GetPipeline()->Use(commandBuffer, index);

	cameraResourceTable->Use(commandBuffer, index, 0, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                         VK_PIPELINE_BIND_POINT_COMPUTE);

	mChunkPositionsResourceTable->Use(commandBuffer, index, 1, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                                  VK_PIPELINE_BIND_POINT_COMPUTE);

	mIndexedIndirectResourceTable->Use(commandBuffer, index, 2, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	                                   VK_PIPELINE_BIND_POINT_COMPUTE);

	vkCmdDispatch(commandBuffer[index], TOTAL_VERTEX_PAGE_COUNT, 1, 1);
}

void phx::World::Draw(VkCommandBuffer* commandBuffer, uint32_t index)
{
	RenderTechnique* standardMaterial = mResourceManager->GetResource<RenderTechnique>("StandardMaterial");

	standardMaterial->GetPipeline()->Use(commandBuffer, index);

	// todo find a way of auto binding global data for shaders, perhaps a global and local mapping
	mResourceManager->GetResource<ResourceTable>("CameraResourceTable")
	    ->Use(commandBuffer, index, 0, standardMaterial->GetPipelineLayout()->GetPipelineLayout());
	mResourceManager->GetResource<ResourceTable>("SamplerArrayResourceTable")
	    ->Use(commandBuffer, index, 1, standardMaterial->GetPipelineLayout()->GetPipelineLayout());

	for (int i = 0; i < TOTAL_VERTEX_PAGE_COUNT; i++)
	{
		VkDeviceSize offsets[] = {sizeof(VertexData) * VERTEX_PAGE_SIZE * i};

		vkCmdBindVertexBuffers(commandBuffer[index], 0, 1, &mVertexBuffer->GetBuffer(), offsets);

		VkDeviceSize positionOffsets[] = {sizeof(glm::mat4) * i};
		// Position data
		vkCmdBindVertexBuffers(commandBuffer[index], 1, 1, &mPositionBuffer->GetBuffer(), positionOffsets);
		vkCmdDrawIndirect(commandBuffer[index], mIndirectDrawCommands->GetBuffer(), sizeof(VkDrawIndirectCommand) * i, 1,
		                  sizeof(VkDrawIndirectCommand));
	}

	RenderTechnique* skybox = mResourceManager->GetResource<RenderTechnique>("Skybox");

	skybox->GetPipeline()->Use(commandBuffer, index);

	mResourceManager->GetResource<ResourceTable>("CameraResourceTable")
	    ->Use(commandBuffer, index, 0, skybox->GetPipelineLayout()->GetPipelineLayout());
	mResourceManager->GetResource<ResourceTable>("SkyboxResourceTable")
	    ->Use(commandBuffer, index, 1, skybox->GetPipelineLayout()->GetPipelineLayout());

	// Vertices are hard baked into the shader.
	vkCmdDraw(commandBuffer[index], 6, 1, 0, 0);
}

phx::VertexPage* phx::World::GetFreeVertexPage()
{
	VertexPage* next = mFreeVertexPages;
	if (mFreeVertexPages)
	{
		mFreeVertexPages  = mFreeVertexPages->next;
		next->next        = nullptr;
		next->vertexCount = 0;

		mFreeMemoryPoolCount--;
	}
	return next;
}

unsigned int phx::World::GetFreeMemoryPoolCount() { return mFreeMemoryPoolCount; }

void phx::World::DestroyBlockFromView()
{
	const float        placeRange = 6.0f;
	const unsigned int stepCount  = 20;
	const float        stepSize   = placeRange / stepCount;

	Chunk* chunk;
	int    localX;
	int    localY;
	int    localZ;
	RaycastToBlock(stepSize, stepCount, chunk, localX, localY, localZ, RaycastMode::Destroy);

	if (chunk)
	{
		chunk->SetBlock(localX, localY, localZ, 0);

		chunk->MarkDirty();

		phx::ChunkNeighbours* nabours = chunk->GetNabours();

		for (int j = 0; j < 6; j++)
		{
			if (nabours->neighbouringChunks[j] == nullptr)
				continue;

			(*nabours->neighbouringChunks[j])->MarkDirty();
		}
	}
}

void phx::World::PlaceBlockFromView()
{
	constexpr ChunkBlock stone = {0x00010001};

	const float        placeRange = 6.0f;
	const unsigned int stepCount  = 20;
	const float        stepSize   = placeRange / stepCount;

	Chunk* chunk;
	int    localX;
	int    localY;
	int    localZ;
	RaycastToBlock(stepSize, stepCount, chunk, localX, localY, localZ, RaycastMode::Place);

	if (chunk)
	{
		chunk->SetBlock(localX, localY, localZ, stone);

		chunk->MarkDirty();

		phx::ChunkNeighbours* nabours = chunk->GetNabours();

		for (int j = 0; j < 6; j++)
		{
			if (nabours->neighbouringChunks[j] == nullptr)
				continue;

			(*nabours->neighbouringChunks[j])->MarkDirty();
		}
	}
}

void phx::World::RaycastToBlock(float step, int iteration, Chunk*& chunkReturn, int& localX, int& localY, int& localZ, RaycastMode mode)
{
	chunkReturn = nullptr;
	localX      = 0;
	localX      = 0;

	Camera*   camera       = mResourceManager->GetResource<Camera>("Camera");
	glm::vec3 viewPosition = camera->GetPosition();

	// Todo in the future we know we will only ever be around the middle chunk, so no need to loop through them all

	Chunk* chunk = nullptr;

	// Find the starting chunk
	for (int i = 0; i < MAX_CHUNKS; i++)
	{
		Chunk* next = &mChunks[i];

		if (PointToCube(viewPosition, next->GetPosition(), CHUNK_BLOCK_SIZE))
		{
			chunk = next;
			break;
		}
	}

	// Check we found a chunk we are starting in
	if (chunk != nullptr)
	{
		glm::ivec3 lastPos = viewPosition;

		int    lastLocalX = 0;
		int    lastLocalY = 0;
		int    lastLocalZ = 0;
		Chunk* lastChunk  = nullptr;

		float        currentStep   = step;
		unsigned int stepHalfCount = 0;

		for (int i = 0; i < iteration; i++)
		{
			viewPosition += camera->GetDirection() * currentStep;
			glm::ivec3 newPos = viewPosition;

			if (viewPosition.x < 0)
				newPos.x -= 1;
			if (viewPosition.y < 0)
				newPos.y -= 1;
			if (viewPosition.z < 0)
				newPos.z -= 1;

			if (newPos != lastPos)
			{
				bool matchingAxies[3] = {newPos.x == lastPos.x, newPos.y == lastPos.y, newPos.z == lastPos.z};

				int matchingAxiesCount = 0;
				for (int j = 0; j < 3; j++)
				{
					if (matchingAxies[j])
					{
						matchingAxiesCount++;
					}
				}

				// If we have managed to get a block on a diaganal, 1 or axies of the cube will be touching the other one
				if (matchingAxiesCount < 2)
				{
					// Half the current step size to try and catch the mixxing blocks
					currentStep *= 0.5f;
					// Since we are splitting a single step into two, we must add two back to the intteration count
					i -= 2;
					if (stepHalfCount < 2)
					{
						stepHalfCount++;
						continue;
					}

					for (int j = matchingAxiesCount; j < 2; j++)
					{
						if (!matchingAxies[0])
						{
							matchingAxies[0] = true;
							newPos.x         = lastPos.x;
						}
						else if (!matchingAxies[2])
						{
							matchingAxies[2] = true;
							newPos.z         = lastPos.z;
						}
						else if (!matchingAxies[1])
						{
							matchingAxies[1] = true;
							newPos.y         = lastPos.y;
						}
						matchingAxiesCount++;
					}
				}
				stepHalfCount = 0;
				currentStep   = step;

				// make sure after raycasting we are no longer outside the chunk, if we are find the new chunk
				if (!PointToCube(viewPosition, chunk->GetPosition(), CHUNK_BLOCK_SIZE))
				{
					Chunk*                newChunk = nullptr;
					phx::ChunkNeighbours* nabours  = chunk->GetNabours();
					for (int j = 0; j < 6; j++)
					{
						if (nabours->neighbouringChunks[j] == nullptr)
							continue;

						if (PointToCube(viewPosition, (*nabours->neighbouringChunks[j])->GetPosition(), CHUNK_BLOCK_SIZE))
						{
							newChunk = *nabours->neighbouringChunks[j];
							break;
						}
					}
					// If we did not find a new chunk, we are off the edge of the map, break
					if (newChunk == nullptr)
						return;

					chunk = newChunk;
				}

				unsigned int blockX = newPos.x;
				unsigned int blockY = newPos.y;
				unsigned int blockZ = newPos.z;

				// Get local block index
				blockX %= CHUNK_BLOCK_SIZE;
				blockY %= CHUNK_BLOCK_SIZE;
				blockZ %= CHUNK_BLOCK_SIZE;

				ChunkBlock block = chunk->GetBlock(blockX, blockY, blockZ);

				// If the block is not air
				if (block != ModHandler::GetAirBlock())
				{
					// If we are destroying a block, respond with the block we are wanting to destroy
					if (mode == RaycastMode::Destroy)
					{
						chunkReturn = chunk;
						localX      = blockX;
						localY      = blockY;
						localZ      = blockZ;
						return;
					}
					// If we are in place mode, respond with the expected air block information
					if (mode == RaycastMode::Place)
					{
						chunkReturn = lastChunk;
						localX      = lastLocalX;
						localY      = lastLocalY;
						localZ      = lastLocalZ;
						return;
					}
				}
				else
				{
					lastLocalX = blockX;
					lastLocalY = blockY;
					lastLocalZ = blockZ;
					lastChunk  = chunk;
				}

				lastPos = newPos;
			}
		}
	}
}

void phx::World::UpdateAllIndirectDraws()
{
	mIndirectDrawCommands->TransferInstantly(mIndirectBufferCPU.get(), sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT);
}

void phx::World::UpdateAllPositionBuffers()
{
	mPositionBuffer->TransferInstantly(mPositionBufferCPU.get(), sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT);
}

void phx::World::ProcessVertexPages(VertexPage* pages, glm::mat4 position)
{
	while (pages != nullptr)
	{

		VkDrawIndirectCommand& indirectCommandInstance = mIndirectBufferCPU.get()[pages->index];
		indirectCommandInstance.vertexCount            = pages->vertexCount;
		indirectCommandInstance.instanceCount          = 1;

		// Transfer the indirect draw request
		mIndirectDrawCommands->TransferInstantly(&indirectCommandInstance, sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));

		mPositionBufferCPU.get()[pages->index] = position;
		mPositionBuffer->TransferInstantly(&mPositionBufferCPU.get()[pages->index], sizeof(glm::mat4), sizeof(glm::mat4) * pages->index);

		pages = pages->next;
	}
	UpdateAllPositionBuffers();
}

void phx::World::FreeVertexPages(VertexPage* pages)
{
	VertexPage* next = nullptr;
	while (pages != nullptr)
	{
		next = pages->next;

		VkDrawIndirectCommand& indirectCommandInstance = mIndirectBufferCPU.get()[pages->index];
		indirectCommandInstance.vertexCount            = 0;
		indirectCommandInstance.instanceCount          = 0;

		// Transfer the indirect draw request
		mIndirectDrawCommands->TransferInstantly(&mIndirectBufferCPU.get()[pages->index], sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));

		pages->next      = mFreeVertexPages;
		mFreeVertexPages = pages;

		mFreeMemoryPoolCount++;
		pages = next;
	}
}
