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

#include <Phoenix/WorldRenderer.hpp>
#include <Phoenix/Chunk.hpp>
#include <Phoenix/World.hpp>

#include <Renderer/Device.hpp>
#include <Renderer/Buffer.hpp>

#include <ResourceManager/ResourceManager.hpp>

static uint32_t GetIndex3D(uint32_t x, uint32_t y, uint32_t z, uint32_t axisLength) { return (z * axisLength + y) * axisLength + x; }

phx::WorldRenderer::WorldRenderer(RenderDevice* renderDevice, ResourceManager* resourceManager, World* world)
    : m_renderDevice(renderDevice), m_resourceManager(resourceManager), m_world(world)
{
}

void phx::WorldRenderer::SetViewRadius(uint32_t radius)
{
	if (radius == m_viewRadius)
		return;

	if (radius == 0)
	{
		m_activeView.reset();
		m_viewRadius = 0;
		m_viewSize   = 0;

		return;
	}

	const bool isNewLarger = radius > m_viewRadius;

	const uint32_t newViewLength = radius * 2 + 1;
	const uint32_t newViewSize   = newViewLength * newViewLength * newViewLength;

	m_activeView = std::make_unique<RenderData[]>(newViewSize);

	// We completely refresh the view for now.
	// Ideally we would simply construct a view using the neighbour system and avoid the chunk lookups.

	for (uint32_t x = 0; x < newViewLength; ++x)
	{
		for (uint32_t y = 0; y < newViewLength; ++y)
		{
			for (uint32_t z = 0; z < newViewLength; ++z)
			{
				// Convert these x, y, z positions into real world positions.
				glm::ivec3 renderPosition = {x, y, z};
				renderPosition -= radius;
				renderPosition *= CHUNK_BLOCK_SIZE;

				glm::ivec3 worldPosition = renderPosition + m_centralPosition;

				Chunk* chunkPtr = m_world->GetChunk(worldPosition);
				if (chunkPtr == nullptr)
				{
					chunkPtr = m_world->AddChunk(worldPosition);
				}

				const uint32_t index = GetIndex3D(x, y, z, newViewLength);

				m_activeView[index].vertexPage     = nullptr;
				m_activeView[index].chunk          = chunkPtr;
				m_activeView[index].renderPosition = renderPosition;
				m_activeView[index].renderMatrix   = glm::translate(glm::mat4(1.f), static_cast<glm::vec3>(renderPosition));
			}
		}
	}

	m_viewRadius = radius;
	m_viewSize   = newViewSize;
}

uint32_t phx::WorldRenderer::GetViewRadius() const { return m_viewRadius; }

uint32_t phx::WorldRenderer::GetViewSize() const { return m_viewSize; }

phx::RenderData* phx::WorldRenderer::GetView() const { return m_activeView.get(); }

void phx::WorldRenderer::Update(const glm::ivec3& position)
{
	glm::ivec3 steppedPosition = position;
	steppedPosition /= CHUNK_BLOCK_SIZE;
	steppedPosition *= CHUNK_BLOCK_SIZE;

	if (steppedPosition == m_lastSteppedPosition)
		return;

	// View is in different position, update.
	glm::ivec3 distance = steppedPosition - m_lastSteppedPosition;
	distance /= CHUNK_BLOCK_SIZE;

	const auto absDistance = glm::abs(distance);
	if (absDistance.x > 1 || absDistance.y > 1 || absDistance.z > 1)
	{
		// @todo Simply rebuild view in this case.
		assert(false && "Too much movement within a single frame.");
	}

	int signX = glm::sign(distance.x);
	int signY = glm::sign(distance.y);
	int signZ = glm::sign(distance.z);

	auto viewLength = m_viewRadius * 2 + 1;

	for (uint32_t i = 0; i < m_viewSize; ++i)
	{
		RenderData* renderData = &m_activeView[i];

		bool changes = false;

		if (signX != 0)
		{
			if (renderData->chunk == nullptr)
			{
				renderData->chunk = m_world->AddChunk(static_cast<glm::ivec3>(renderData->renderPosition) + m_centralPosition);
			}

			renderData->chunk = renderData->chunk->GetNeighbours()->neighbours[signX > 0 ? Chunk::EAST : Chunk::WEST];
			changes           = true;
		}

		if (signY != 0)
		{
			if (renderData->chunk == nullptr)
			{
				renderData->chunk = m_world->AddChunk(static_cast<glm::ivec3>(renderData->renderPosition) + m_centralPosition);
			}

			renderData->chunk = renderData->chunk->GetNeighbours()->neighbours[signY > 0 ? Chunk::TOP : Chunk::BOTTOM];
			changes           = true;
		}

		if (signZ != 0)
		{
			if (renderData->chunk == nullptr)
			{
				renderData->chunk = m_world->AddChunk(static_cast<glm::ivec3>(renderData->renderPosition) + m_centralPosition);
			}

			renderData->chunk = renderData->chunk->GetNeighbours()->neighbours[signZ > 0 ? Chunk::SOUTH : Chunk::NORTH];
			changes           = true;
		}
	}
}
