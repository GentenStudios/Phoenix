#pragma once

#include <Globals/Globals.hpp>

class Buffer;

namespace phx
{
	struct VertexData
	{
		glm::vec3 position;
		glm::vec2 uv;
	};

	class Chunk
	{
	public:
		Chunk();
		~Chunk() = default;

		void Update();

		void         SetVertexMemory(Buffer* buffer, unsigned int offset);
		unsigned int GetVertexCount();

		uint64_t GetBlock(int x, int y, int z);
		void     SetBlock(int x, int y, int z, uint64_t block);

	private:
		void GenerateMesh();

	private:
		unsigned int m_vertexCount        = 0;
		unsigned int m_vertexBufferOffset = 0;
		Buffer*      m_vertexBuffer       = nullptr;

		bool m_dirty = true;

		uint64_t m_blocks[CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE];
	};
} // namespace phx
