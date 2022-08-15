// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#define GLM_ENABLE_EXPERIMENTAL
//#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/rotate_vector.hpp>


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
const unsigned int MAX_WORLD_CHUNKS_PER_AXIS = 7;

// Total chunks in memory at once
const unsigned int MAX_CHUNKS = MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS * MAX_WORLD_CHUNKS_PER_AXIS;

const unsigned int VERTEX_PAGE_SIZE = 24 * 200;

const unsigned int TOTAL_VERTEX_PAGE_COUNT = 1000;

