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
