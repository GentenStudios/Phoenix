#include <Phoenix/World.hpp>

#include <Phoenix/Chunk.hpp>
#include <Phoenix/Collision.hpp>
#include <Phoenix/Mods.hpp>
#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/ResourceTable.hpp>
#include <Renderer/ResourceTableLayout.hpp>
#include <Renderer/Camera.hpp>

#include <ResourceManager/ResourceManager.hpp>
#include <ResourceManager/RenderTechnique.hpp>

phx::World::World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager)
    : mDevice(device), mResourceManager(resourceManager)
{
	mVertexBuffer = std::unique_ptr<Buffer>(
	    new Buffer(mDevice, memoryHeap, VERTEX_PAGE_SIZE * sizeof(VertexData) * TOTAL_VERTEX_PAGE_COUNT,
	               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	               VK_SHARING_MODE_EXCLUSIVE));

	mIndirectDrawCommands = std::unique_ptr<Buffer>(
	    new Buffer(mDevice, memoryHeap, sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT, VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	               VK_SHARING_MODE_EXCLUSIVE));

	mPositionBuffer = std::unique_ptr<Buffer>(
	    new Buffer(mDevice, memoryHeap, sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
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

	mChunks = new Chunk[MAX_CHUNKS];
	mChunksSorted = new Chunk*[MAX_CHUNKS];
	mChunkNeighbours = new ChunkNabours[MAX_CHUNKS];

	mFreeVertexPages = nullptr;

	mPositionBufferCPU = std::unique_ptr<glm::mat4>(new glm::mat4[TOTAL_VERTEX_PAGE_COUNT]);
	mVertexPages = std::unique_ptr<VertexPage>(new VertexPage[TOTAL_VERTEX_PAGE_COUNT]);

	for (int i = TOTAL_VERTEX_PAGE_COUNT - 1; i >= 0; --i)
	{
		mPositionBufferCPU.get()[i] = glm::mat4(1.0f);

		mVertexPages.get()[i].index = i;
		mVertexPages.get()[i].offset = (VERTEX_PAGE_SIZE * sizeof(VertexData)) * i;
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

				ChunkNabours* chunkNabours = &mChunkNeighbours[index];

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

//mIndirectDrawCommands
mIndexedIndirectResourceTable->Use(commandBuffer, index, 2, frustrumPipeline->GetPipelineLayout()->GetPipelineLayout(),
	VK_PIPELINE_BIND_POINT_COMPUTE);

	//vkCmdDispatch(commandBuffer[index], TOTAL_VERTEX_PAGE_COUNT, 1, 1);
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
		VkDeviceSize offsets[] = { sizeof(VertexData) * VERTEX_PAGE_SIZE * i };

		vkCmdBindVertexBuffers(commandBuffer[index], 0, 1, &mVertexBuffer->GetBuffer(), offsets);

		VkDeviceSize positionOffsets[] = { sizeof(glm::mat4) * i };
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
	vkCmdDraw(commandBuffer[index], 36, 1, 0, 0);
}

phx::VertexPage* phx::World::GetFreeVertexPage()
{
	VertexPage* next = mFreeVertexPages;
	if (mFreeVertexPages)
	{
		mFreeVertexPages = mFreeVertexPages->next;
		next->next = nullptr;
		next->vertexCount = 0;

		mFreeMemoryPoolCount--;
	}
	return next;
}

unsigned int phx::World::GetFreeMemoryPoolCount() { return mFreeMemoryPoolCount; }

void phx::World::DestroyBlockFromView()
{
	const float		   placeRange = 6.0f;
	const unsigned int stepCount = 20;
	const float        stepSize = placeRange / stepCount;

	Chunk* chunk;
	int localX;
	int localY;
	int localZ;
	RaycastToBlock(stepSize, stepCount, chunk, localX, localY, localZ, RaycastMode::Destroy);

	if (chunk)
	{
		chunk->SetBlock(localX, localY, localZ, 0);

		chunk->MarkDirty();

		phx::ChunkNabours* nabours = chunk->GetNabours();

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

		phx::ChunkNabours* nabours = chunk->GetNabours();

		for (int j = 0; j < 6; j++)
		{
			if (nabours->neighbouringChunks[j] == nullptr)
				continue;

			(*nabours->neighbouringChunks[j])->MarkDirty();
		}
	}

}

void phx::World::RaycastToBlock(float step, int itteration, Chunk*& chunkReturn, int& localX, int& localY, int& localZ, RaycastMode mode)
{
	chunkReturn            = nullptr; 
	localX                 = 0;
	localX                 = 0;

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

		int lastLocalX = 0;
		int lastLocalY = 0;
		int lastLocalZ = 0;
		Chunk* lastChunk = nullptr;

		float currentStep = step;
		unsigned int stepHalfCount = 0;

		for (int i = 0; i < itteration; i++)
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
							newPos.x = lastPos.x;
						}
						else if (!matchingAxies[2])
						{
							matchingAxies[2] = true;
							newPos.z = lastPos.z;
						}
						else if (!matchingAxies[1])
						{
							matchingAxies[1] = true;
							newPos.y = lastPos.y;
						}
						matchingAxiesCount++;
					}
				}
				stepHalfCount = 0;
				currentStep = step;

				// make sure after raycasting we are no longer outside the chunk, if we are find the new chunk
				if (!PointToCube(viewPosition, chunk->GetPosition(), CHUNK_BLOCK_SIZE))
				{
					Chunk*             newChunk = nullptr;
					phx::ChunkNabours* nabours  = chunk->GetNabours();
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
				if ( block != ModHandler::GetAirBlock())
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
	while(pages != nullptr)
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


		pages->next = mFreeVertexPages;
		mFreeVertexPages = pages;

		mFreeMemoryPoolCount++;
		pages = next;
	}

}
