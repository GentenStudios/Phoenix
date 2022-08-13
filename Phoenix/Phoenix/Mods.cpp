#include <Phoenix/Mods.hpp>

#include <pugixml.hpp>

#include <cassert>
#include <iostream>
#include <filesystem>

phx::ModHandler::ModHandler(unsigned modCount)
{
	// We need to add 1 to the modCount because we have a default mod.
	++modCount;

	m_mods        = new Mod[modCount];

	// We have 2 default blocks, core.air and core.unknown.
	// aka ID's 0 and 1 in Mod 0.

	Mod core;
	core.blocks.AllocateMemory(2);

	core.blocks.AddBlock(/*core.air*/);
	core.blocks.AddBlock(/*core.unknown*/);

	core.lookupIndex = 0;

	m_mods[0] = core;
}

phx::ModHandler::~ModHandler()
{
	delete[] m_mods;
}

void phx::ModHandler::AddMod(const std::filesystem::path& modXMLPath)
{
	// Parse mod xml, etc...
	pugi::xml_document doc;
	if (!doc.load_file(modXMLPath.c_str()))
	{
		std::cout << "Failed to load mod in " << modXMLPath << "\n";
		return;
	}

	const pugi::xml_node data = doc.child("Mod");

	const pugi::xml_attribute modName = data.attribute("name");
	if (modName)
	{
		std::cout << "The mod configuration file at: " << modXMLPath << " is invalid. No name is defined.\n";
		return;
	}

	Mod mod;
	mod.lookupIndex = m_currentLookupIndex;
	mod.modName     = modName.as_string();

	const auto         blocks = data.children("Blocks");
	const unsigned int count  = std::distance(blocks.begin(), blocks.end());

	mod.blocks.AllocateMemory(count);
	for (const auto& block : blocks)
	{
		const auto blockName = block.attribute("name");
		if (!blockName)
		{
			std::cout << "The mod: " << mod.modName << " has an invalid block entry.\n";
			continue;
		}

		// Singular texture for now.
		const auto blockTexture = block.attribute("texture");
		if (!blockTexture)
		{
			std::cout << "The mod: " << mod.modName << " has an invalid block entry.\n";
			continue;
		}

		mod.blocks.AddBlock();
	}


	m_mods[m_currentLookupIndex] = mod;

	++m_currentLookupIndex;
}

phx::Mod* phx::ModHandler::GetMod(int lookupIndex) const
{
	assert(lookupIndex < m_currentLookupIndex);

	return &m_mods[lookupIndex];
}
