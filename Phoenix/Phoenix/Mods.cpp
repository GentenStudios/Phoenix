#include <Phoenix/Mods.hpp>

#include <pugixml.hpp>

#include <cassert>
#include <filesystem>
#include <iostream>

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
