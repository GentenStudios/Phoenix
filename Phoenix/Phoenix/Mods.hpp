#pragma once

#include <Phoenix/Blocks.hpp>

#include <string>
#include <filesystem>

namespace phx
{
	struct Mod
	{
		uint16_t lookupIndex;

		// todo replace with pointer to a string pool or similar.
		std::string modName;

		BlockHandler blocks;
	};

	class ModHandler
	{
	public:
		explicit ModHandler(unsigned int modCount);
		~ModHandler();

		void AddMod(const std::filesystem::path& modXMLPath);

		Mod* GetMod(int lookupIndex) const;

	private:
		Mod*      m_mods;
		uint16_t  m_currentLookupIndex = 0;
	};
} // namespace phx
