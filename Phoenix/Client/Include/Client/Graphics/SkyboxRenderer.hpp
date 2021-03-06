// Copyright 2019-20 Genten Studios
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its contributors
// may be used to endorse or promote products derived from this software without
// specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <Client/Graphics/ShaderPipeline.hpp>

#include <Common/Position.hpp>

#include <entt/entity/registry.hpp>
#include <string>
#include <vector>

namespace phx::gfx
{
	// currently only renders skybox, will do more in the future.
	class SkyboxRenderer
	{
	public:
		SkyboxRenderer() = default;
		~SkyboxRenderer() = default;

		// give in front, left, back, right, top, bottom
		void setSkyboxTextures(const std::vector<std::string>& textures);
		/**
		 * @brief Renders the skybox for a particular frame
		 * @param position Position of the entity the skybox is being rendered for
		 * @param projection Calculated projection matrix
		 * @param dt Time since the last frame
		 */
		void tick(entt::registry* registry, entt::entity entity, const math::mat4& projection, const float& dt);

	private:
		bool         m_initialTick   = true;
		bool         m_enabled       = false;

		unsigned int   m_skyboxTex;
		unsigned int   m_skyboxVao;
		unsigned int   m_skyboxVbo;
		ShaderPipeline m_skyboxPipeline;
	};
} // namespace phx::gfx
