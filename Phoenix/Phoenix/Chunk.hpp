#pragma once

#include <Globals/Globals.hpp>

class Buffer;

struct VertexData
{
	glm::vec3 position;
	glm::vec2 uv;
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

	uint64_t mBlocks[CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE];

	bool mDirty;
};
