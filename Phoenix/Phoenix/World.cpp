#include <World.hpp>

#include <Chunk.hpp>
#include <device.hpp>
#include <buffer.hpp>
#include <devicememory.hpp>
#include <resourcemanager.hpp>
#include <pipeline.hpp>
#include <pipelinelayout.hpp>
#include <resourcetable.hpp>

#include <rendertechnique.hpp>

World::World(RenderDevice* device, MemoryHeap* memoryHeap, ResourceManager* resourceManager) : mDevice(device), mResourceManager(resourceManager)
{
	mVertexBuffer = std::unique_ptr<Buffer>(new Buffer(
		mDevice, memoryHeap, MAX_VERTECIES_PER_CHUNK * sizeof(glm::vec3) * MAX_CHUNKS,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
	));

	mIndirectDrawCommands = std::unique_ptr<Buffer>(new Buffer(
		mDevice, memoryHeap, sizeof(VkDrawIndirectCommand) * MAX_CHUNKS,
		VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
	));

	mPositionBuffer = std::unique_ptr<Buffer>(new Buffer(
		mDevice, memoryHeap, sizeof(glm::mat4) * MAX_CHUNKS,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
	));

	mIndirectBufferCPU = std::unique_ptr<VkDrawIndirectCommand>(new VkDrawIndirectCommand[MAX_CHUNKS]);

	VkDrawIndirectCommand indirectCommandInstance{};
	indirectCommandInstance.vertexCount = 0;
	indirectCommandInstance.instanceCount = 0;
	indirectCommandInstance.firstVertex = 0;
	indirectCommandInstance.firstInstance = 0;

	UpdateAllIndirectDraws();

	for (uint32_t i = 0; i < MAX_CHUNKS; i++)
	{
		mIndirectBufferCPU.get()[i] = indirectCommandInstance;
	}

	mChunks = new Chunk[MAX_CHUNKS];


	mPositionBufferCPU = std::unique_ptr<glm::mat4>(new glm::mat4[MAX_CHUNKS]);

	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].SetVertexMemory(mVertexBuffer.get(), MAX_VERTECIES_PER_CHUNK * sizeof(glm::vec3) * i);

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

		modelMatrix = glm::translate(modelMatrix, glm::vec3(x,y,z));

		mPositionBufferCPU.get()[i] = modelMatrix;
	}

	// Temp, need to be updated dynaicly
	UpdateAllPositionBuffers();
}

World::~World()
{
	mVertexBuffer.reset();

	delete[] mChunks;
}

void World::Update()
{

	for (int i = 0; i < MAX_CHUNKS; ++i)
	{
		mChunks[i].Update();

		// To do check for has changed

		VkDrawIndirectCommand& indirectCommandInstance = mIndirectBufferCPU.get()[i];
		indirectCommandInstance.vertexCount = mChunks[i].GetVertexCount();
		indirectCommandInstance.instanceCount = 1;
		indirectCommandInstance.firstVertex = 0;
		indirectCommandInstance.firstInstance = 0;

		// Transfer the indirect draw request
		mIndirectDrawCommands->TransferInstantly(mIndirectBufferCPU.get(), sizeof(VkDrawIndirectCommand), i * sizeof(VkDrawIndirectCommand));
	}
}

void World::Draw(VkCommandBuffer* commandBuffer, uint32_t index)
{
	RenderTechnique* standardMaterial = mResourceManager->GetResource<RenderTechnique>("StandardMaterial");

	standardMaterial->GetPipeline()->Use(commandBuffer, index);

	// todo find a way of auto binding global data for shaders, prehaps a global and local mapping
	mResourceManager->GetResource<ResourceTable>("CameraResourceTable")->Use(commandBuffer, index, 0, standardMaterial->GetPipelineLayout()->GetPipelineLayout());
	mResourceManager->GetResource<ResourceTable>("SamplerArrayResourceTable")->Use(commandBuffer, index, 1, standardMaterial->GetPipelineLayout()->GetPipelineLayout());


	VkDeviceSize offsets[] = { 0 };

	vkCmdBindVertexBuffers(
		commandBuffer[index],
		0,
		1,
		&mVertexBuffer->GetBuffer(),
		offsets
	);

	for (int i = 0; i < MAX_CHUNKS; i++)
	{
		VkDeviceSize positionOffsets[] = { sizeof(glm::mat4) * i };
		// Position data
		vkCmdBindVertexBuffers(
			commandBuffer[index],
			1,
			1,
			&mPositionBuffer->GetBuffer(),
			positionOffsets
		);
		vkCmdDrawIndirect(
			commandBuffer[index],
			mIndirectDrawCommands->GetBuffer(),
			sizeof(VkDrawIndirectCommand) * i,
			1,
			sizeof(VkDrawIndirectCommand)
		);
	}

}

void World::UpdateAllIndirectDraws()
{
	mIndirectDrawCommands->TransferInstantly(mIndirectBufferCPU.get(), sizeof(VkDrawIndirectCommand) * MAX_CHUNKS);
}

void World::UpdateAllPositionBuffers()
{
	mPositionBuffer->TransferInstantly(mPositionBufferCPU.get(), sizeof(glm::mat4) * MAX_CHUNKS);
}
