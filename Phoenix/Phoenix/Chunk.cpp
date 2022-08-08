#include <Phoenix/Chunk.hpp>
#include <Phoenix/World.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/DeviceMemory.hpp>

#include <Globals/Globals.hpp>

phx::Chunk::Chunk()
{
	m_vertexPage = nullptr;
	Reset();
}

// Temp mesh
// clang-format off
const glm::vec3 BLOCK_VERTICES[] = {
    {0.f, 1.f, 1.f}, // east (right)
	{0.f, 1.f, 0.f},
	{0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
	{0.f, 0.f, 1.f},
	{0.f, 1.f, 1.f},

    {1.f, 1.f, 1.f}, // west
    {1.f, 1.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 0.f},
	{1.f, 0.f, 1.f},
	{1.f, 1.f, 1.f},

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

    {1.f, 1.f, 0.f}, // north (front)
	{1.f, 0.f, 0.f},
	{0.f, 0.f, 0.f}, 
    {0.f, 0.f, 0.f},
	{0.f, 1.f, 0.f},
	{1.f, 1.f, 0.f},

    {1.f, 1.f, 1.f}, // south
    {1.f, 0.f, 1.f},
	{0.f, 0.f, 1.f}, 
	{0.f, 0.f, 1.f},
	{0.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
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
	{1.f, 0.f},
	{0.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{1.f, 1.f},

    {1.f, 0.f},
	{0.f, 0.f},
	{0.f, 1.f},
	{0.f, 1.f},
	{1.f, 1.f},
	{1.f, 0.f},
};
// clang-format on

void phx::Chunk::Initilize(World* world, Buffer* vertexBuffer)
{
	m_vertexBuffer = vertexBuffer;
	m_world = world;
}

void phx::Chunk::SetPosition(glm::vec3 position)
{
	m_position = position;

	m_matrix = glm::mat4(1.0f);
	m_matrix = glm::translate(m_matrix, m_position);

	// Temp : generate the worlds data
	GenerateWorld();
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
				m_blocks[i][j][k] = 0;
			}
		}
	}
}

void phx::Chunk::GenerateWorld()
{
	for (int k = 0; k < CHUNK_BLOCK_SIZE; ++k)
	{
		for (int j = 0; j < CHUNK_BLOCK_SIZE; ++j)
		{
			for (int i = 0; i < CHUNK_BLOCK_SIZE; ++i)
			{
				float y = (float) j + m_position.y;
				// Randomly place blocks for now
				m_blocks[i][j][k] = y < 0 ? 1 : 0;
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

uint64_t phx::Chunk::GetBlock(int x, int y, int z) { return m_blocks[x][y][z]; }

void phx::Chunk::SetBlock(int x, int y, int z, uint64_t block) { m_blocks[x][y][z] = block; }

void phx::Chunk::GenerateMesh()
{
	int totalVertexCount = 0;

	void*       memoryPtr    = nullptr;
	VertexData* vertexStream = nullptr;



	m_world->FreeVertexPages(m_vertexPage);
	m_vertexPage = nullptr;


	for (int x = 0; x < CHUNK_BLOCK_SIZE; ++x)
	{
		for (int y = 0; y < CHUNK_BLOCK_SIZE; ++y)
		{
			for (int z = 0; z < CHUNK_BLOCK_SIZE; ++z)
			{
				// Check if we are about to render air
				uint64_t blockID = m_blocks[x][y][z];
				if (blockID == 0)
					continue;

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

					m_vertexBuffer->GetDeviceMemory()->UnMap();

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


				bool visibilitySet[6] = {
				    (x == 0) || (m_blocks[x - 1][y][z] == 0), (x == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x + 1][y][z] == 0),
				    (y == 0) || (m_blocks[x][y - 1][z] == 0), (y == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x][y + 1][z] == 0),
				    (z == 0) || (m_blocks[x][y][z - 1] == 0), (z == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x][y][z + 1] == 0),
				};

				// Loop through for all faces of the block
				for (int j = 0; j < 6; j++)
				{
					if (visibilitySet[j])
					{
						// Temp texture solution
						int faceTextureID = blockID - 1;
						// Loop through for the face vertices
						for (int k = 0; k < 6; k++)
						{

							unsigned int lookupIndex = k + (j * 6);

							(*vertexStream).position = BLOCK_VERTICES[lookupIndex];
							(*vertexStream).position.x += x;
							(*vertexStream).position.y += y;
							(*vertexStream).position.z += z;

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
		m_vertexBuffer->GetDeviceMemory()->UnMap();
	}

	m_world->ProcessVertexPages(m_vertexPage, m_matrix);
}
