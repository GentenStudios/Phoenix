#pragma once

#include <globals.hpp>

class Buffer;

class Chunk
{
public:
	Chunk();

	~Chunk();

	void Update();

	void SetVertexMemory(Buffer* buffer, unsigned int offset);

	unsigned int GetVertexCount();

private:

	void GenerateMesh();

	unsigned int mVertexCount;
	unsigned int mVertexBufferOffset;
	Buffer* mVertexBuffer;

	bool mDirty;
};