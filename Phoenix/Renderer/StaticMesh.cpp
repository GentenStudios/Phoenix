#include <Renderer/Buffer.hpp>
#include <Renderer/Device.hpp>
#include <Renderer/DeviceMemory.hpp>
#include <Renderer/MemoryHeap.hpp>
#include <Renderer/StaticMesh.hpp>

StaticMesh::StaticMesh(RenderDevice* device, MemoryHeap* memoryHeap, char* vertexData, uint32_t vertexDataSize, char* indexData,
                       uint32_t indexDataSize, uint32_t indexCount)
    : m_device(device), m_indexCount(indexCount)
{
	m_vertexBuffer = std::make_unique<Buffer>(m_device, memoryHeap, vertexDataSize,
	                                          VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
	                                          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                                          VK_SHARING_MODE_EXCLUSIVE);

	m_vertexBuffer->TransferInstantly(vertexData, vertexDataSize);

	m_indexBuffer = std::make_unique<Buffer>(m_device, memoryHeap, indexDataSize,
	                                         VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
	                                         VK_BUFFER_USAGE_TRANSFER_DST_BIT,
	                                         VK_SHARING_MODE_EXCLUSIVE);

	m_indexBuffer->TransferInstantly(indexData, indexDataSize);
}

void StaticMesh::Use(VkCommandBuffer* commandBuffer, uint32_t index) const
{
	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(commandBuffer[index], 0, 1, &m_vertexBuffer->GetBuffer(), offsets);
	vkCmdBindIndexBuffer(commandBuffer[index], m_indexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void StaticMesh::Draw(VkCommandBuffer* commandBuffer, uint32_t index) const { vkCmdDrawIndexed(commandBuffer[index], m_indexCount, 1, 0, 0, 0); }
