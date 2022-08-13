#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

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
		std::string displayName;

		std::string  texture;
		unsigned int textureIndex = 0;
	};

	class BlockHandler
	{
	public:
		BlockHandler() = default;
		~BlockHandler() = default;

		BlockHandler(const BlockHandler&) = delete;
		BlockHandler& operator=(const BlockHandler&) = delete;

		BlockHandler(BlockHandler&& other);
		BlockHandler& operator=(BlockHandler&& other);

		void AllocateMemory(unsigned int blockCount);

		uint16_t AddBlock(const std::string& name, const std::string& displayName, const std::string& texture);

		Block* GetBlock(uint16_t lookupIndex) const;
		Block* GetBlock(const std::string& name) const;

	private:
		std::unique_ptr<Block[]> m_blocks;
		uint16_t                 m_currentLookupIndex = 0;

		std::unordered_map<std::string, uint16_t> m_blockNameLookup;
	};
} // namespace phx
