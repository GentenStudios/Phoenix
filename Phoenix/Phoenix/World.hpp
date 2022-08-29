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

#include <Phoenix/Chunk.hpp>

#include <glm/gtx/hash.hpp>

#include <memory>
#include <unordered_map>
#include <mutex>
#include <shared_mutex>
#include <queue>

class RenderDevice;
class ResourceManager;

namespace phx
{
	class World
	{
	public:
		enum class Action
		{
			SET,
			PLACE,
			BREAK
		};

	public:
		World(RenderDevice* renderDevice, ResourceManager* resourceManager);
		~World();

		void Update(const glm::ivec3& position);
		void Draw();

		Chunk**  GetView() const;
		uint32_t GetViewSize() const;

		uint32_t GetViewRadius() const;
		void     SetViewRadius(uint32_t radius);

		Chunk* GetChunk(const glm::ivec3& position) const;
		Chunk* AddChunk(const glm::ivec3& position);

		ChunkBlock GetBlock(const glm::ivec3& position) const;
		void       SetBlock(const glm::ivec3& position, ChunkBlock block, Action action = Action::SET);

	private:
		RenderDevice*    m_renderDevice;
		ResourceManager* m_resourceManager;

		std::shared_mutex                     m_mapMutex;
		std::unordered_map<glm::ivec3, Chunk> m_chunkMap;

		glm::ivec3                m_lastViewPosition = {0, 0, 0};
		uint32_t                  m_viewRadius       = 0;
		uint32_t                  m_viewSize         = 0;
		std::unique_ptr<Chunk*[]> m_activeView;

		WorldRenderer* m_worldRenderer;

		std::mutex             m_generationMutex;
		std::queue<glm::ivec3> m_generationQueue;

		std::thread m_generationWorker;
	};
} // namespace phx
