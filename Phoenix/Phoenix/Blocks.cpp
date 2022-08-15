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

#include <Phoenix/Blocks.hpp>

#include <cassert>

phx::BlockHandler::BlockHandler(BlockHandler&& other)
{
	std::swap(m_blocks, other.m_blocks);
	std::swap(m_currentLookupIndex, other.m_currentLookupIndex);
	std::swap(m_blockNameLookup, other.m_blockNameLookup);
}

phx::BlockHandler& phx::BlockHandler::operator=(BlockHandler&& other)
{
	std::swap(m_blocks, other.m_blocks);
	std::swap(m_currentLookupIndex, other.m_currentLookupIndex);
	std::swap(m_blockNameLookup, other.m_blockNameLookup);

	return *this;
}

void phx::BlockHandler::AllocateMemory(unsigned blockCount) { m_blocks = std::unique_ptr<Block[]>(new Block[blockCount]); }

uint16_t phx::BlockHandler::AddBlock(const std::string& name, const std::string& displayName, const std::string& texture)
{
	Block block;
	block.lookupIndex = m_currentLookupIndex;
	block.name        = name;
	block.displayName = displayName;
	block.texture     = texture;

	m_blocks[m_currentLookupIndex] = block;
	m_blockNameLookup[name]        = m_currentLookupIndex;

	return m_currentLookupIndex++;
}

phx::Block* phx::BlockHandler::GetBlock(uint16_t lookupIndex) const
{
	if (lookupIndex >= m_currentLookupIndex)
		return nullptr;

	return &m_blocks[lookupIndex];
}

phx::Block* phx::BlockHandler::GetBlock(const std::string& name) const
{
	const auto it = m_blockNameLookup.find(name);
	if (it == m_blockNameLookup.end())
		return nullptr;

	return &m_blocks[it->second];
}

uint16_t phx::BlockHandler::GetBlockCount() const { return m_currentLookupIndex; }

phx::Block* phx::BlockHandler::GetBlocks() const { return m_blocks.get(); }

