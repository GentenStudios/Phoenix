#include <Renderer/StaticMesh.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/Buffer.hpp>
#include <Renderer/MemoryHeap.hpp>



StaticMesh::StaticMesh( RenderDevice* device, MemoryHeap* memoryHeap, char* vertexData, uint32_t vertexDataSize, char* indexData, uint32_t indexDataSize, uint32_t indexCount ) : mDevice( device ), mIndexCount( indexCount )
{
	mVertexBuffer = std::unique_ptr<Buffer>( new Buffer(
		mDevice, memoryHeap, vertexDataSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
	) );

	mVertexBuffer->TransferInstantly( vertexData, vertexDataSize );

	mIndexBuffer = std::unique_ptr<Buffer>( new Buffer(
		mDevice, memoryHeap, indexDataSize,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_SHARING_MODE_EXCLUSIVE
	) );

	mIndexBuffer->TransferInstantly( indexData, indexDataSize );
}

StaticMesh::~StaticMesh( )
{
}

void StaticMesh::Use( VkCommandBuffer* commandBuffer, uint32_t index )
{
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(
		commandBuffer[index],
		0,
		1,
		&mVertexBuffer->GetBuffer( ),
		offsets
	);
	vkCmdBindIndexBuffer(
		commandBuffer[index],
		mIndexBuffer->GetBuffer( ),
		0,
		VK_INDEX_TYPE_UINT32
	);
}

void StaticMesh::Draw( VkCommandBuffer* commandBuffer, uint32_t index )
{

	vkCmdDrawIndexed(
		commandBuffer[index],
		mIndexCount,
		1,
		0,
		0,
		0
	);
}
