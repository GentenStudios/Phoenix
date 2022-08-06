#include <Phoenix/Chunk.hpp>

#include <Renderer/Buffer.hpp>
#include <Renderer/DeviceMemory.hpp>

Chunk::Chunk()
{
	mVertexBufferOffset = 0;
	mVertexBuffer = nullptr;
	mVertexCount = 0;

    mDirty = true;

    // Temp random blocks
    for (int i = 0; i < CHUNK_BLOCK_SIZE; ++i)
        for (int j = 0; j < CHUNK_BLOCK_SIZE; ++j)
            for (int k = 0; k < CHUNK_BLOCK_SIZE; ++k)
            {
                // Randomly place blocks for now
                mBlocks[i][j][k] = 1;// rand() % 4 == 0 ? 1 : 0;
            }

}

Chunk::~Chunk()
{
}

// Temp mesh
const glm::vec3 BLOCK_VERTICIES[] =
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

void Chunk::Update()
{
    if(mDirty)
    {
        GenerateMesh();
        mDirty = false;
    }
}

void Chunk::SetVertexMemory(Buffer* buffer, unsigned int offset)
{
	mVertexBuffer = buffer;
	mVertexBufferOffset = offset;
}

unsigned int Chunk::GetVertexCount()
{
	return mVertexCount;
}

uint64_t Chunk::GetBlock(int x, int y, int z)
{
    return mBlocks[x][y][z];
}

void Chunk::SetBlock(int x, int y, int z, uint64_t block)
{
    mBlocks[x][y][z] = block;
}

void Chunk::GenerateMesh()
{
    int vertexCount = 0;

    void* memoryPtr = nullptr;
    mVertexBuffer->GetDeviceMemory()->Map(
        MAX_VERTECIES_PER_CHUNK * sizeof(VertexData),
        mVertexBuffer->GetMemoryOffset() + mVertexBufferOffset,
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
                if (mBlocks[x][y][z] == 0)
                    continue;


                bool visibilitySet[6] =
                {
                    (x == 0) || (mBlocks[x - 1][y][z] == 0),
                    (x == CHUNK_BLOCK_SIZE - 1) || (mBlocks[x + 1][y][z] == 0),
                    (y == 0) || (mBlocks[x][y - 1][z] == 0),
                    (y == CHUNK_BLOCK_SIZE - 1) || (mBlocks[x][y + 1][z] == 0),
                    (z == 0) || (mBlocks[x][y][z - 1] == 0),
                    (z == CHUNK_BLOCK_SIZE - 1) || (mBlocks[x][y][z + 1] == 0),
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

                            (*vertexStream).position = BLOCK_VERTICIES[lookupIndex];
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

    mVertexCount = vertexCount;


    mVertexBuffer->GetDeviceMemory()->UnMap();
}
