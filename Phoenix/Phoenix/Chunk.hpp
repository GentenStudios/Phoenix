#pragma once

#include <Globals/Globals.hpp>

#include <Phoenix/Blocks.hpp>

#include <memory>

class Buffer;

namespace phx
{
	class World;
	struct VertexPage;
	struct VertexData
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 uv;
		uint32_t textureID;
	};

	class ModHandler;
	class Chunk;

	struct ChunkNeighbours
	{
		Chunk** neighbouringChunks[6];
	};

	class Chunk
	{

	public:

		enum Face
		{
			East,
			West,
			Top,
			Bottom,
			North,
			South,
		};

		Chunk();
		~Chunk() = default;

		void Initialize(World* world, Buffer* vertexBuffer, ModHandler* modHandler);

		void SetPosition(glm::ivec3 position);

		void SetRenderPosition(glm::vec3 position);

		void Reset();

		void GenerateWorld();

		void Update();

		unsigned int GetTotalVertexCount();

		void SetNeighbouringChunk(ChunkNeighbours* neighbouringChunks);

		ChunkBlock GetBlock(int x, int y, int z);
		void     SetBlock(int x, int y, int z, ChunkBlock block);

		void MarkDirty();

		glm::ivec3 GetPosition();

		ChunkNeighbours* GetNabours();


	private:
		void GenerateMesh();

	private:
		World*       m_world;
		unsigned int m_totalVertexCount = 0;
		Buffer*      m_vertexBuffer       = nullptr;

		ModHandler* m_modHandler;

		ChunkNeighbours* m_neighbouringChunk;

		VertexPage* m_vertexPage;

		bool m_dirty = true;

		glm::ivec3 m_position;
		glm::mat4 m_matrix;

		ChunkBlock m_blocks[CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE][CHUNK_BLOCK_SIZE];
	};
} // namespace phx
