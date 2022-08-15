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

