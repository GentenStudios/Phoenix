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
			} val;

			uint64_t id;
		};

		constexpr ChunkBlock() : id(0) {}
		constexpr ChunkBlock(uint64_t block) : id(block) {}
		constexpr ChunkBlock(uint16_t mod, uint16_t block, uint32_t data) : val({mod, block, data}) {}
	};

	inline bool operator==(const ChunkBlock& lhs, const ChunkBlock& rhs) { return lhs.id == rhs.id; }
	inline bool operator!=(const ChunkBlock& lhs, const ChunkBlock& rhs) { return lhs.id != rhs.id; }

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

		uint16_t GetBlockCount() const;
		Block*   GetBlocks() const;

	private:
		std::unique_ptr<Block[]> m_blocks;
		uint16_t                 m_currentLookupIndex = 0;

		std::unordered_map<std::string, uint16_t> m_blockNameLookup;
	};
} // namespace phx

