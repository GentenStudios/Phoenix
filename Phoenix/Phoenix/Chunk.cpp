#include <Chunk.hpp>

#include <buffer.hpp>
#include <devicememory.hpp>

Chunk::Chunk()
{
	mVertexBufferOffset = 0;
	mVertexBuffer = nullptr;
	mVertexCount = 0;

}

Chunk::~Chunk()
{
}

void Chunk::Update()
{
	const int vertexCount = 6;

	// Temp mesh
	glm::vec3 vertcies[vertexCount] =
	{
		glm::vec3(-0.5f,-0.5f,0.0f),
		glm::vec3(0.5f,-0.5f,0.0f),
		glm::vec3(-0.5f,0.5f,0.0f),

		glm::vec3(0.5f,0.5f,0.0f),
		glm::vec3(0.5f,-0.5f,0.0f),
		glm::vec3(-0.5f,0.5f,0.0f)

	};

	mVertexCount = vertexCount;

	mVertexBuffer->TransferInstantly(vertcies, vertexCount * sizeof(glm::vec3), mVertexBufferOffset);

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
