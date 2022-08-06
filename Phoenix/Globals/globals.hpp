#pragma once


#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

const unsigned int MAX_SPRITESHEET_SAMPLER_ARRAY = 32;


// How wide the chunk is in blocks
const unsigned int CHUNK_BLOCK_SIZE = 16;

// How many chunks are visible in x,y,z range around the camera ((n/2) + 1 rad)
const unsigned int MAX_WORLD_CHUNKS_PER_AXIS = 11;

// Total chunks in memory at once
const unsigned int MAX_CHUNKS = MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS;


const unsigned int MAX_VERTECIES_PER_CHUNK = 10000;

