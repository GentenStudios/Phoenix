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

class RenderDevice;
class ResourceManager;

namespace phx
{
	struct VertexPage
	{
		uint32_t          index;
		uint32_t          offset;
		uint32_t          vertexCount;
		VertexPage*       next = nullptr;
	};

	struct ChunkRenderData
	{
		glm::ivec3 worldPosition;

		VertexMemoryPage* vertexPage;
		glm::vec3         renderPosition;
		glm::mat4         renderMatrix;

		Chunk* chunk;
	};

	class WorldRenderer
	{
	public:
		WorldRenderer(RenderDevice* renderDevice, ResourceManager* resourceManager);
		~WorldRenderer();

		void     SetViewRadius(uint32_t radius);
		void     GetViewRadius() const;
		uint32_t GetViewSize() const;
		Chunk**  GetView() const;

		void Update(const glm::ivec3& position);
		void Draw();

	private:
		void FrustumCull();
		void OrderDraws();

	private:
		RenderDevice*    m_renderDevice;
		ResourceManager* m_resourceManager;

		glm::ivec3                m_centralPosition;
		uint32_t                  m_viewRadius = 0;
		uint32_t                  m_viewSize   = 0;
		std::unique_ptr<Chunk*[]> m_activeView;


		glm::ivec3 m_lastSteppedPosition;
	};
} // namespace phx
