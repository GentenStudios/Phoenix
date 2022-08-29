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

#pragma once

#include <Renderer/Vulkan.hpp>

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

#include <memory>

class RenderDevice;
class ResourceManager;
class Buffer;

namespace phx
{
	class World;
	class Chunk;

	struct VertexPage
	{
		uint32_t    index;
		uint32_t    offset;
		uint32_t    vertexCount;
		VertexPage* next = nullptr;
	};

	struct RenderData
	{
		VertexPage* vertexPage;
		Chunk*      chunk;

		glm::vec3 renderPosition;
		glm::mat4 renderMatrix;
	};

	class WorldRenderer
	{
	public:
		WorldRenderer(RenderDevice* renderDevice, ResourceManager* resourceManager, World* world);
		~WorldRenderer() = default;

		void        SetViewRadius(uint32_t radius);
		uint32_t    GetViewRadius() const;
		uint32_t    GetViewSize() const;
		RenderData* GetView() const;

		void Update(const glm::ivec3& position);
		void Draw();

	private:
		void FrustumCull();
		void OrderDraws();

	private:
		RenderDevice*    m_renderDevice;
		ResourceManager* m_resourceManager;
		World*           m_world;

		glm::ivec3                    m_centralPosition;
		uint32_t                      m_viewRadius = 0;
		uint32_t                      m_viewSize   = 0;
		std::unique_ptr<RenderData[]> m_activeView;

		std::unique_ptr<VertexPage[]> m_vertexPages;
		VertexPage*                   m_freeVertexPage = nullptr;

		std::unique_ptr<glm::mat4>                      m_renderPositions;
		std::unique_ptr<VkDrawIndexedIndirectCommand[]> m_indirectCommands;

		std::unique_ptr<Buffer> m_indirectBuffer;
		std::unique_ptr<Buffer> m_positionBuffer;

		glm::ivec3 m_lastSteppedPosition = {0, 0, 0};
	};
} // namespace phx
