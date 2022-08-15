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

#include <Phoenix/Mods.hpp>

#include <pugixml.hpp>

#include <filesystem>
#include <iostream>
#include <cassert>
#include <cstring>

phx::ModHandler::ModHandler(unsigned modCount)
{
	// We need to add 1 to the modCount because we have a default mod.
	++modCount;

	m_mods = std::unique_ptr<Mod[]>(new Mod[modCount]);

	// We have 2 default blocks, core.air and core.unknown.
	// aka ID's 0 and 1 in Mod 0.

	Mod* core = &m_mods[m_currentLookupIndex];
	core->blocks.AllocateMemory(2);

	core->blocks.AddBlock("core.air", "Air", "");
	core->blocks.AddBlock("core.unknown", "Unknown", "");

	core->lookupIndex = 0;

	++m_currentLookupIndex;
}

phx::Block* phx::ModHandler::GetBlock(ChunkBlock block) const
{
	const Mod* mod = GetMod(block.val.modID);
	if (!mod)
		return nullptr;

	return mod->blocks.GetBlock(block.val.blockID);
}

phx::Block* phx::ModHandler::GetBlock(const std::string& block) const
{
	// Split by the first period.
	int pos = 0;
	while (block[pos] != '.')
		++pos;

	if (pos == block.size() - 1)
		return nullptr;

	const std::string modName   = block.substr(0, pos);
	const std::string blockName = block.substr(pos + 1);

	const Mod* mod = GetMod(modName);
	if (!mod)
		return nullptr;

	return mod->blocks.GetBlock(blockName);
}

phx::Mod* phx::ModHandler::AddMod(const std::filesystem::path& modXMLPath)
{
	pugi::xml_document doc;
	if (!doc.load_file(modXMLPath.c_str()))
	{
		std::cout << "Failed to load mod in " << modXMLPath << "\n";
		return nullptr;
	}

	const pugi::xml_node data = doc.child("Mod");

	const pugi::xml_attribute modName = data.attribute("name");
	if (!modName)
	{
		std::cout << "The mod configuration file at: " << modXMLPath << " is invalid. No name is defined.\n";
		return nullptr;
	}

	Mod mod;
	mod.lookupIndex = m_currentLookupIndex;
	mod.name        = modName.as_string();

	const auto         blocks = data.child("Blocks");
	const unsigned int count  = std::distance(blocks.begin(), blocks.end());

	mod.blocks.AllocateMemory(count);
	for (const auto& block : blocks)
	{
		const auto blockName        = block.attribute("name");
		const auto blockDisplayName = block.attribute("displayName");
		const auto blockTexture     = block.attribute("texture");

		if (!blockName || !blockDisplayName || !blockTexture)
		{
			std::cout << "The mod: " << mod.name << " has an invalid block entry.\n";
			continue;
		}

		auto fixedTexturePath = modXMLPath.parent_path();
		fixedTexturePath /= blockTexture.as_string();

		mod.blocks.AddBlock(blockName.as_string(), blockDisplayName.as_string(), fixedTexturePath.string());
	}

	// Blocks are loaded, now let's see if they want to register a skybox.
	const auto skybox = data.child("Skybox");
	if (skybox)
	{
		// We currently ignore the "name" attribute, but eventually we want to let people perhaps select a skybox or have different skybox's
		// for different scenarios.
		const auto textures = skybox.children("Texture");
		const auto texCount = std::distance(textures.begin(), textures.end());

		if (texCount == 6)
		{
			m_skyboxTextures.resize(6);
			for (auto& tex : textures)
			{
				// Skybox textures need to be in order:
				// east, west, top, bottom, north, south
				const auto position = tex.attribute("position");

				// @todo improve logic here.
				int index = 0;
				if (std::strcmp(position.as_string(), "east") == 0)
				{
					index = 0;
				}
				else if (std::strcmp(position.as_string(), "west") == 0)
				{
					index = 1;
				}
				else if (std::strcmp(position.as_string(), "top") == 0)
				{
					index = 2;
				}
				else if (std::strcmp(position.as_string(), "bottom") == 0)
				{
					index = 3;
				}
				else if (std::strcmp(position.as_string(), "north") == 0)
				{
					index = 4;
				}
				else if (std::strcmp(position.as_string(), "south") == 0)
				{
					index = 5;
				}
				else
				{
					std::cout << "The mod: " << mod.name << " has an invalid skybox entry.\n";
				}

				const auto localPath = tex.attribute("texture").as_string();

				auto fixedTexturePath = modXMLPath.parent_path();
				fixedTexturePath /= localPath;

				m_skyboxTextures[index] = fixedTexturePath.string();
			}
		}
		else
		{
			std::cout << "The mod: " << mod.name << " has an invalid block entry.\n";
		}
	}

	m_modNameLookup[mod.name] = m_currentLookupIndex;
	m_mods[m_currentLookupIndex] = std::move(mod);

	return &m_mods[m_currentLookupIndex++];
}

phx::Mod* phx::ModHandler::GetMod(int lookupIndex) const
{
	if (lookupIndex >= m_currentLookupIndex)
		return nullptr;

	return &m_mods[lookupIndex];
}

phx::Mod* phx::ModHandler::GetMod(const std::string& modName) const
{
	const auto it = m_modNameLookup.find(modName);
	if (it == m_modNameLookup.end())
		return nullptr;

	return &m_mods[it->second];
}

uint16_t phx::ModHandler::GetModCount() { return m_currentLookupIndex; }

phx::Mod* phx::ModHandler::GetMods() { return m_mods.get(); }

const std::vector<std::string>& phx::ModHandler::GetSkyboxTextures() const { return m_skyboxTextures; }

