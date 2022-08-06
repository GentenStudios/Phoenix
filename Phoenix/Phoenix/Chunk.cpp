#include <Phoenix/Chunk.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/DeviceMemory.hpp>

phx::Chunk::Chunk()
{
    // Temp random blocks
    for (int i = 0; i < CHUNK_BLOCK_SIZE; ++i)
    {
	    for (int j = 0; j < CHUNK_BLOCK_SIZE; ++j)
	    {
		    for (int k = 0; k < CHUNK_BLOCK_SIZE; ++k)
		    {
			    // Randomly place blocks for now
			    m_blocks[i][j][k] = 1;// rand() % 4 == 0 ? 1 : 0;
		    }
	    }
    }
}

// Temp mesh
const glm::vec3 BLOCK_VERTICES[] =
{
    {0.f, 1.f, 1.f},
    {0.f, 1.f, 0.f},
    {0.f, 0.f, 0.f}, // east (right)
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


    {1.f, 1.f, 0.f},
    {1.f, 0.f, 0.f},
    {0.f, 0.f, 0.f}, // north (front)
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

        {1.f,  0.f},
        {0.f,  0.f},
        {0.f,  1.f},
        {0.f,  1.f},
        {1.f,  1.f},
        {1.f,  0.f},

        {0.f,  0.f},
        {0.f,  1.f},
        {1.f,  1.f},
        {1.f,  1.f},
        {1.f,  0.f},
        {0.f,  0.f},

        {1.f,  1.f},
        {1.f,  0.f},
        {0.f,  0.f},
        {0.f,  0.f},
        {0.f,  1.f},
        {1.f,  1.f},

        {0.f,  1.f},
        {1.f,  1.f},
        {1.f,  0.f},
        {1.f,  0.f},
        {0.f,  0.f},
        {0.f,  1.f},

        {1.f,  1.f},
        {1.f,  0.f},
        {0.f,  0.f},
        {0.f,  0.f},
        {0.f,  1.f},
        {1.f,  1.f},

        {1.f,  0.f},
        {0.f,  0.f},
        {0.f,  1.f},
        {0.f,  1.f},
        {1.f,  1.f},
        {1.f,  0.f},
};

void phx::Chunk::Update()
{
    if(m_dirty)
    {
        GenerateMesh();
        m_dirty = false;
    }
}

void phx::Chunk::SetVertexMemory(Buffer* buffer, unsigned int offset)
{
	m_vertexBuffer = buffer;
	m_vertexBufferOffset = offset;
}

unsigned int phx::Chunk::GetVertexCount()
{
	return m_vertexCount;
}

uint64_t phx::Chunk::GetBlock(int x, int y, int z)
{
    return m_blocks[x][y][z];
}

void phx::Chunk::SetBlock(int x, int y, int z, uint64_t block)
{
    m_blocks[x][y][z] = block;
}

void phx::Chunk::GenerateMesh()
{
    int vertexCount = 0;

    void* memoryPtr = nullptr;
    m_vertexBuffer->GetDeviceMemory()->Map(
        MAX_VERTICES_PER_CHUNK * sizeof(VertexData),
        m_vertexBuffer->GetMemoryOffset() + m_vertexBufferOffset,
        memoryPtr
    );

    VertexData* vertexStream = reinterpret_cast<VertexData*>(memoryPtr);

    for (int x = 0; x < CHUNK_BLOCK_SIZE; ++x)
    {
        for (int y = 0; y < CHUNK_BLOCK_SIZE; ++y)
        {
            for (int z = 0; z < CHUNK_BLOCK_SIZE; ++z)
            {
                // Check if we are about to render air
                if (m_blocks[x][y][z] == 0)
                    continue;


                bool visibilitySet[6] =
                {
                    (x == 0) || (m_blocks[x - 1][y][z] == 0),
                    (x == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x + 1][y][z] == 0),
                    (y == 0) || (m_blocks[x][y - 1][z] == 0),
                    (y == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x][y + 1][z] == 0),
                    (z == 0) || (m_blocks[x][y][z - 1] == 0),
                    (z == CHUNK_BLOCK_SIZE - 1) || (m_blocks[x][y][z + 1] == 0),
                };

                // Loop through for all faces of the block
                for (int j = 0; j < 6; j++)
                {
                    if (visibilitySet[j])
                    {
                        // Loop through for the face vertices
                        for (int k = 0; k < 6; k++)
                        {

                            unsigned int lookupIndex = k + (j * 6);

                            (*vertexStream).position = BLOCK_VERTICES[lookupIndex];
                            (*vertexStream).position.x += x;
                            (*vertexStream).position.y += y;
                            (*vertexStream).position.z += z;


                            (*vertexStream).uv = BLOCK_UVS[lookupIndex];


                            vertexStream++;

                        }

                        vertexCount += 6;
                    }
                }


            }
        }
    }

    m_vertexCount = vertexCount;


    m_vertexBuffer->GetDeviceMemory()->UnMap();
}
