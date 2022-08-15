#include <Phoenix/Chunk.hpp>
#include <Phoenix/Phoenix.hpp>
#include <Phoenix/World.hpp>
#include <Phoenix/Mods.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/DeviceMemory.hpp>

#include <Globals/Globals.hpp>
#include <ResourceManager/ResourceManager.hpp>

phx::Chunk::Chunk()
{
	m_vertexPage = nullptr;

	m_neighbouringChunk = nullptr;

	Reset();
}

// Temp mesh
// clang-format off
const glm::vec3 BLOCK_VERTICES[] = {
    {1.f, 1.f, 1.f}, // east (right)
    {1.f, 1.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 1.f},
	{1.f, 1.f, 1.f},

	{0.f, 0.f, 0.f}, // west
	{0.f, 1.f, 0.f},
    {0.f, 1.f, 1.f}, 
	{0.f, 1.f, 1.f},
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 0.f},

    {1.f, 0.f, 1.f}, // bottom
    {1.f, 0.f, 0.f},
	{0.f, 0.f, 0.f},
	{0.f, 0.f, 0.f},
	{0.f, 0.f, 1.f},
	{1.f, 0.f, 1.f},

    {0.f, 1.f, 0.f}, // top
    {1.f, 1.f, 0.f},
	{1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
	{0.f, 1.f, 1.f},
	{0.f, 1.f, 0.f},
	
	{0.f, 0.f, 0.f},  // north (front)
	{1.f, 0.f, 0.f},
    {1.f, 1.f, 0.f},
	{1.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
    {0.f, 0.f, 0.f},

    {1.f, 1.f, 1.f}, // south
    {1.f, 0.f, 1.f},
	{0.f, 0.f, 1.f}, 
	{0.f, 0.f, 1.f},
	{0.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
};
const glm::vec3 BLOCK_NORMALS[] = {
	

    {1.f, 0.f, 0.f}, // east (right)
    {1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},

	{1.f, 0.f, 0.f}, // west
	{1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f}, 
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f},

    {0.f, 1.f, 0.f}, // bottom
    {0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},

    {0.f, 1.f, 0.f}, // top
    {0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	{0.f, 1.f, 0.f},
	
	{0.f, 0.f, 1.f},  // north (front)
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},

    {0.f, 0.f, 1.f}, // south
    {0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f}, 
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
	{0.f, 0.f, 1.f},
};

static const glm::vec2 BLOCK_UVS[] = {

    {1.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},

    {0.f, 0.f},
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
	{0.f, 0.f},

    {1.f, 1.f},
	{1.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{1.f, 1.f},

    {0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
	{1.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},

	{1.f, 1.f},
	{0.f, 1.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{1.f, 0.f},
    {1.f, 1.f},
	
	{1.f, 0.f},
	{1.f, 1.f},
	{0.f, 1.f},
	{0.f, 1.f},
	{0.f, 0.f},
    {1.f, 0.f},
};
// clang-format on

void phx::Chunk::Initialize(World* world, Buffer* vertexBuffer, ModHandler* modHandler)
{
	m_vertexBuffer = vertexBuffer;
	m_world = world;
	m_modHandler   = modHandler;
}

void phx::Chunk::SetPosition(glm::ivec3 position)
{
	m_position = position;
}

void phx::Chunk::SetRenderPosition(glm::vec3 position)
{
	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, position);
}

void phx::Chunk::Reset()
{
	// Temp random blocks
	for (int k = 0; k < CHUNK_BLOCK_SIZE; ++k)
	{
		for (int j = 0; j < CHUNK_BLOCK_SIZE; ++j)
		{
			for (int i = 0; i < CHUNK_BLOCK_SIZE; ++i)
			{
				m_blocks[i][j][k] = ModHandler::GetAirBlock();
			}
		}
	}
}

void phx::Chunk::GenerateWorld()
{

	for (int z = 0; z < CHUNK_BLOCK_SIZE; ++z)
	{
		for (int y = 0; y < CHUNK_BLOCK_SIZE; ++y)
		{
			for (int x = 0; x < CHUNK_BLOCK_SIZE; ++x)
			{
				float ya = (float) y + m_position.y;
	
				bool solid = ya < -5;

				// hard coded blocks.
				constexpr ChunkBlock dirt  = {0x00000001};
				constexpr ChunkBlock stone = {0x00010001};

				m_blocks[x][y][z] = solid ? dirt : ModHandler::GetAirBlock();

			}
		}
	}


	m_dirty = true;
}

void phx::Chunk::Update()
{
	if (m_dirty)
	{
		GenerateMesh();
		m_dirty = false;
	}
}

unsigned int phx::Chunk::GetTotalVertexCount() { return m_totalVertexCount; }

void phx::Chunk::SetNeighbouringChunk(ChunkNeighbours* neighbouringChunk) { m_neighbouringChunk = neighbouringChunk; }

phx::ChunkBlock phx::Chunk::GetBlock(int x, int y, int z) { return m_blocks[x][y][z]; }

void phx::Chunk::SetBlock(int x, int y, int z, ChunkBlock block)
{
	m_blocks[x][y][z] = block;
	m_dirty           = true;
}

void phx::Chunk::MarkDirty() { m_dirty = true; }

glm::ivec3 phx::Chunk::GetPosition() { return m_position; }

phx::ChunkNeighbours* phx::Chunk::GetNabours() { return m_neighbouringChunk; }

void phx::Chunk::GenerateMesh()
{
	int totalVertexCount = 0;

	void*       memoryPtr    = nullptr;
	VertexData* vertexStream = nullptr;

	m_world->FreeVertexPages(m_vertexPage);
	m_vertexPage = nullptr;

	if (m_neighbouringChunk == nullptr)
		return;

	for (int x = 0; x < CHUNK_BLOCK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_BLOCK_SIZE; ++y)
		{
			for (int z = 0; z < CHUNK_BLOCK_SIZE; ++z)
			{
				// Check if we are about to render air
				ChunkBlock blockID = m_blocks[x][y][z];
				if (blockID == ModHandler::GetAirBlock())
					continue;
		
				bool visibilitySet[6] = {false, false, false, false, false, false};

				// East
				if (x == CHUNK_BLOCK_SIZE - 1)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::East];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::East] = (*neighbor)->m_blocks[0][y][z] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::East] = (m_blocks[x + 1][y][z] == ModHandler::GetAirBlock());
				}
				
				// West
				if (x == 0)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::West];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::West] = (*neighbor)->m_blocks[CHUNK_BLOCK_SIZE - 1][y][z] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::West] = (m_blocks[x - 1][y][z] == ModHandler::GetAirBlock());
				}
				
				// Top
				if (y == 0)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::Top];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::Top] = (*neighbor)->m_blocks[x][CHUNK_BLOCK_SIZE - 1][z] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::Top] = (m_blocks[x][y - 1][z] == ModHandler::GetAirBlock());
				}
				
				// Bottom
				if (y == CHUNK_BLOCK_SIZE - 1)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::Bottom];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::Bottom] = (*neighbor)->m_blocks[x][0][z] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::Bottom] = (m_blocks[x][y + 1][z] == ModHandler::GetAirBlock());
				}
				
				
				// North
				if (z == 0)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::North];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::North] = (*neighbor)->m_blocks[x][y][CHUNK_BLOCK_SIZE - 1] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::North] = (m_blocks[x][y][z - 1] == ModHandler::GetAirBlock());
				}
				
				
				// South
				if (z == CHUNK_BLOCK_SIZE - 1)
				{
					Chunk** neighbor = m_neighbouringChunk->neighbouringChunks[Chunk::South];
					if (neighbor != nullptr)
					{
						visibilitySet[Chunk::South] = (*neighbor)->m_blocks[x][y][0] == ModHandler::GetAirBlock();
					}
				}
				else
				{
					visibilitySet[Chunk::South] = (m_blocks[x][y][z + 1] == ModHandler::GetAirBlock());
				}


				// Loop through for all faces of the block
				for (int j = 0; j < 6; j++)
				{
					if (visibilitySet[j])
					{
						if (m_vertexPage == nullptr)
						{
							// Allocate the first page

							m_vertexPage = m_world->GetFreeVertexPage();

							if (m_vertexPage == nullptr)
							{
								assert(0 && "To do, no more pages");
								return;
							}

							m_vertexBuffer->GetDeviceMemory()->Map(VERTEX_PAGE_SIZE * sizeof(VertexData),
							                                       m_vertexBuffer->GetMemoryOffset() + m_vertexPage->offset, memoryPtr);

							vertexStream = reinterpret_cast<VertexData*>(memoryPtr);
						}
						else if (VERTEX_PAGE_SIZE - m_vertexPage->vertexCount < 36)
						{
							// Move onto a new page

							m_vertexBuffer->GetDeviceMemory()->Unmap();

							VertexPage* newPage = m_world->GetFreeVertexPage();

							if (newPage == nullptr)
							{
								assert(0 && "To do, no more pages");
								return;
							}

							m_vertexBuffer->GetDeviceMemory()->Map(VERTEX_PAGE_SIZE * sizeof(VertexData),
							                                       m_vertexBuffer->GetMemoryOffset() + newPage->offset, memoryPtr);

							newPage->next = m_vertexPage;
							m_vertexPage  = newPage;

							vertexStream = reinterpret_cast<VertexData*>(memoryPtr);
						}

						// Temp texture solution
						int faceTextureID = m_modHandler->GetBlock(blockID)->textureIndex;
						// Loop through for the face vertices
						for (int k = 0; k < 6; k++)
						{
							unsigned int lookupIndex = k + (j * 6);

							(*vertexStream).position = BLOCK_VERTICES[lookupIndex];
							(*vertexStream).position.x += x;
							(*vertexStream).position.y += y;
							(*vertexStream).position.z += z;

							(*vertexStream).normal = BLOCK_NORMALS[lookupIndex];							

							(*vertexStream).uv = BLOCK_UVS[lookupIndex];

							(*vertexStream).textureID = faceTextureID;

							vertexStream++;
						}

						m_vertexPage->vertexCount += 6;
						totalVertexCount += 6;
					}
				}
			}
		}
	}

	m_totalVertexCount = totalVertexCount;

	if (m_vertexPage != nullptr)
	{
		m_vertexBuffer->GetDeviceMemory()->Unmap();
	}

	m_world->ProcessVertexPages(m_vertexPage, m_matrix);
}
