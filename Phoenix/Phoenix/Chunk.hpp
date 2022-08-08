#pragma once

#include <Globals/Globals.hpp>

class Buffer;

namespace phx
{
	class World;
	class VertexPage;
	struct VertexData
	{
		glm::vec3 position;
		glm::vec2 uv;
		uint32_t textureID;
	};

	class Chunk
	{

	public:
		Chunk();
		~Chunk() = default;

		void Initilize(World* world, Buffer* vertexBuffer);

		void SetPosition(glm::vec3 position);

		void Reset();

		void GenerateWorld();

		void Update();

		unsigned int GetTotalVertexCount();

		uint64_t GetBlock(int x, int y, int z);
		void     SetBlock(int x, int y, int z, uint64_t block);

	private:
		void GenerateMesh();

	private:
		World*       m_world;
		unsigned int m_totalVertexCount = 0;
		Buffer*      m_vertexBuffer       = nullptr;

		VertexPage* m_vertexPage;

		bool m_dirty = true;

		glm::vec3 m_position;
		glm::mat4 m_matrix;

		uint64_t m_blocks[CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE];
	};
} // namespace phx
