#include <Chunk.hpp>

#include <buffer.hpp>
#include <devicememory.hpp>

Chunk::Chunk()
{
	mVertexBufferOffset = 0;
	mVertexBuffer = nullptr;
	mVertexCount = 0;

    mDirty = true;

}

Chunk::~Chunk()
{
}

// Temp mesh
const glm::vec3 globalVerticies[] =
{
    {1.f, 1.f, 0.f},
    {1.f, 0.f, 0.f},
    {0.f, 0.f, 0.f}, // north (front)
    {0.f, 0.f, 0.f},
    {0.f, 1.f, 0.f},
    {1.f, 1.f, 0.f},

    {0.f, 1.f, 1.f},
    {0.f, 1.f, 0.f},
    {0.f, 0.f, 0.f}, // east (right)
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 1.f},
    {0.f, 1.f, 1.f},

    {1.f, 1.f, 1.f}, // south
    {1.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},
    {0.f, 0.f, 1.f},
    {0.f, 1.f, 1.f},
    {1.f, 1.f, 1.f},

    {1.f, 1.f, 1.f}, // west
    {1.f, 1.f, 0.f},
    {1.f, 0.f, 0.f},
    {1.f, 0.f, 0.f},
    {1.f, 0.f, 1.f},
    {1.f, 1.f, 1.f},

    {0.f, 1.f, 0.f}, // top
    {1.f, 1.f, 0.f},
    {1.f, 1.f, 1.f},
    {1.f, 1.f, 1.f},
    {0.f, 1.f, 1.f},
    {0.f, 1.f, 0.f},

    {1.f, 0.f, 1.f}, // bottom
    {1.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 0.f},
    {0.f, 0.f, 1.f},
    {1.f, 0.f, 1.f},
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

void Chunk::GenerateMesh()
{
    const int vertexCount = MAX_VERTECIES_PER_CHUNK;

    void* memoryPtr = nullptr;
    mVertexBuffer->GetDeviceMemory()->Map(
        MAX_VERTECIES_PER_CHUNK * sizeof(glm::vec3),
        mVertexBuffer->GetMemoryOffset() + mVertexBufferOffset,
        memoryPtr
    );

    glm::vec3* vertexStream = reinterpret_cast<glm::vec3*>(memoryPtr);


    for (int i = 0; i < MAX_BLOCKS_PER_CHUNK; ++i)
    {

        float x = i % CHUNK_BLOCK_SIZE;
        float y = (i / CHUNK_BLOCK_SIZE) % CHUNK_BLOCK_SIZE;
        float z = i / (CHUNK_BLOCK_SIZE * CHUNK_BLOCK_SIZE);


        for (int j = 0; j < 36; j++)
        {


            *vertexStream = globalVerticies[j];
            (*vertexStream).x += x;
            (*vertexStream).y += y;
            (*vertexStream).z += z;

            vertexStream++;
        }



    }


    mVertexCount = vertexCount;


    mVertexBuffer->GetDeviceMemory()->UnMap();
}
