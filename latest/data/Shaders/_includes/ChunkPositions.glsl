
struct ChunkPositions{
	mat4 position;
};

layout(std430, set=1, binding=0) buffer ChunkPositionsBuffer {
		ChunkPositions chunkPositions[];
};