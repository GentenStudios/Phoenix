#pragma once

#include <Globals/globals.hpp>

class Buffer;

struct VertexData
{
	glm::vec3 position;
};

class Chunk
{
public:
	Chunk();

	~Chunk();

	void Update();

	void SetVertexMemory(Buffer* buffer, unsigned int offset);

	unsigned int GetVertexCount();

	uint64_t GetBlock(int x, int y, int z);

	void SetBlock(int x, int y, int z, uint64_t block);

private:

	void GenerateMesh();

	unsigned int mVertexCount;
	unsigned int mVertexBufferOffset;
	Buffer* mVertexBuffer;

	uint64_t mBlocks[MAX_BLOCKS_PER_CHUNK];

	bool mDirty;
};
