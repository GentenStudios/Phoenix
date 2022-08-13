#include <Phoenix/Blocks.hpp>

#include <cassert>

phx::BlockHandler::~BlockHandler() { delete[] m_blocks; }

void phx::BlockHandler::AllocateMemory(unsigned blockCount) { m_blocks = new Block[blockCount]; }

uint16_t phx::BlockHandler::AddBlock()
{
	// We will have block name, etc... at some point to load in.

	Block block;
	block.lookupIndex = m_currentLookupIndex;

	m_blocks[m_currentLookupIndex] = block;

	return m_currentLookupIndex++;
}

phx::Block* phx::BlockHandler::GetBlock(uint16_t lookupIndex) const
{
	assert(lookupIndex < m_currentLookupIndex);

	return &m_blocks[lookupIndex];
}
