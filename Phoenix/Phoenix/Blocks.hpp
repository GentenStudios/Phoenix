#pragma once

#include <cstdint>
#include <string>

namespace phx
{
	struct ChunkBlock
	{
		union
		{
			struct
			{
				uint16_t modID;
				uint16_t blockID;
				uint32_t metadata;
			};

			uint64_t id;
		};
	};

	struct Block
	{
		uint16_t lookupIndex;

		std::string name;
	};

	class BlockHandler
	{
	public:
		BlockHandler() = default;
		~BlockHandler();

		void AllocateMemory(unsigned int blockCount);

		uint16_t AddBlock();

		Block* GetBlock(uint16_t lookupIndex) const;

	private:
		Block*    m_blocks             = nullptr;
		uint16_t  m_currentLookupIndex = 0;
	};
} // namespace phx
