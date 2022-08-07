#include <Phoenix/World.hpp>

#include <Phoenix/Chunk.hpp>
#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/Pipeline.hpp>
#include <Renderer/PipelineLayout.hpp>
#include <Renderer/ResourceTable.hpp>
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
	    new Buffer(mDevice, memoryHeap, sizeof(VkDrawIndirectCommand) * TOTAL_VERTEX_PAGE_COUNT,
	               VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	               VK_SHARING_MODE_EXCLUSIVE));

	mPositionBuffer = std::unique_ptr<Buffer>(
	    new Buffer(mDevice, memoryHeap, sizeof(glm::mat4) * TOTAL_VERTEX_PAGE_COUNT,
	               VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	               VK_SHARING_MODE_EXCLUSIVE));

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

	UpdateAllIndirectDraws();

	mChunks = new Chunk[MAX_CHUNKS];

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

	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].Initilize(this,mVertexBuffer.get());

		glm::mat4 modelMatrix(1.0f);

		float x = (MAX_WORLD_CHUNKS_PER_AXIS / 2);
		float y = (MAX_WORLD_CHUNKS_PER_AXIS / 2);
		float z = (MAX_WORLD_CHUNKS_PER_AXIS / 2);

		x -= i % MAX_WORLD_CHUNKS_PER_AXIS;
		y -= (i / MAX_WORLD_CHUNKS_PER_AXIS) % MAX_WORLD_CHUNKS_PER_AXIS;
		z -= i / (MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS);

		x *= CHUNK_BLOCK_SIZE;
		y *= CHUNK_BLOCK_SIZE;
		z *= CHUNK_BLOCK_SIZE;

		modelMatrix = glm::translate(modelMatrix, glm::vec3(x, y, z));
				
		mChunks[i].SetPosition(modelMatrix);
	}
	UpdateAllPositionBuffers();
}

phx::World::~World()
{
	mVertexBuffer.reset();

	delete[] mChunks;
}

void phx::World::Update()
{

	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].Update();
		UpdateAllPositionBuffers();
		UpdateAllIndirectDraws();
	}
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
}

phx::VertexPage* phx::World::GetFreeVertexPage() 
{ 
	VertexPage* next = mFreeVertexPages;
	if (mFreeVertexPages)
	{
		mFreeVertexPages = mFreeVertexPages->next;
		next->next       = nullptr;
		next->vertexCount = 0;
	}
	return next;
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
		mIndirectDrawCommands->TransferInstantly(mIndirectBufferCPU.get(), sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));

		mPositionBufferCPU.get()[pages->index] = position;
		mPositionBuffer->TransferInstantly(mPositionBufferCPU.get(), sizeof(glm::mat4), sizeof(glm::mat4) * pages->index);

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
		mIndirectDrawCommands->TransferInstantly(mIndirectBufferCPU.get(), sizeof(VkDrawIndirectCommand),
		                                         pages->index * sizeof(VkDrawIndirectCommand));




		pages->next = mFreeVertexPages;
		mFreeVertexPages = pages->next;

		pages = next;
	}

}
