#pragma once


#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const unsigned int MAX_SPRITESHEET_SAMPLER_ARRAY = 32;

// How wide the chunk is in blocks
// Keep this to a power of 2 (2,4,8, etc)
const unsigned int CHUNK_BLOCK_SIZE = 16;

// How many bits make up the chunk block size
const unsigned int CHUNK_BLOCK_BIT_SIZE = 4;

// An AND mask for the size
const unsigned int CHUNK_BLOCK_BIT_SIZE_MASK = 0b1111;

const unsigned int MAX_BLOCKS_PER_CHUNK = CHUNK_BLOCK_SIZE * CHUNK_BLOCK_SIZE * CHUNK_BLOCK_SIZE;

// How many chunks are visible in x,y,z range around the camera ((n/2) + 1 rad)
const unsigned int MAX_WORLD_CHUNKS_PER_AXIS = 5;

// Total chunks in memory at once
const unsigned int MAX_CHUNKS = MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS;

const unsigned int MAX_VERTICES_PER_CHUNK = 36 * CHUNK_BLOCK_SIZE * CHUNK_BLOCK_SIZE * (CHUNK_BLOCK_SIZE / 2);
