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

#include <Phoenix/Blocks.hpp>

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>

namespace phx
{
	struct Mod
	{
		uint16_t    lookupIndex;
		std::string name;

		BlockHandler blocks;
	};

	class ModHandler
	{
	public:
		explicit ModHandler(unsigned int modCount);
		~ModHandler() = default;

		// Helper functions to reduce magic numbers.
		static constexpr ChunkBlock GetAirBlock() { return 0; }
		static constexpr uint16_t   GetCoreModID() { return 0; }
		static constexpr uint16_t   GetAirBlockID() { return 0; }
		static constexpr uint16_t   GetUnknownBlockID() { return 1; }

		Block* GetBlock(ChunkBlock block) const;
		Block* GetBlock(const std::string& block) const;

		Mod* AddMod(const std::filesystem::path& modXMLPath);

		Mod* GetMod(int lookupIndex) const;
		Mod* GetMod(const std::string& modName) const;

		uint16_t GetModCount();
		Mod*     GetMods();

		const std::vector<std::string>& GetSkyboxTextures() const;

	private:
		std::unique_ptr<Mod[]> m_mods;
		uint16_t               m_currentLookupIndex = 0;

		// temporary way of registering skybox textures.
		std::vector<std::string> m_skyboxTextures;

		// @todo Replace this with something more performant.
		std::unordered_map<std::string, uint16_t> m_modNameLookup;
	};
} // namespace phx

