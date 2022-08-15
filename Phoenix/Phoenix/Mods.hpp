#pragma once

#include <Phoenix/Blocks.hpp>

#include <string>
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

	private:
		std::unique_ptr<Mod[]> m_mods;
		uint16_t               m_currentLookupIndex = 0;

		// @todo Replace this with something more performant.
		std::unordered_map<std::string, uint16_t> m_modNameLookup;
	};
} // namespace phx
