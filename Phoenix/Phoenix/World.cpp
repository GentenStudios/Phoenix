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

#include <Phoenix/Chunk.hpp>
#include <Phoenix/World.hpp>
#include <Phoenix/WorldRenderer.hpp>

static uint32_t GetIndex3D(uint32_t x, uint32_t y, uint32_t z, uint32_t axisLength) { return (z * axisLength + y) * axisLength + x; }

phx::Chunk** phx::World::GetView() const { return m_activeView.get(); }

uint32_t phx::World::GetViewSize() const { return m_viewSize; }

uint32_t phx::World::GetViewRadius() const { return m_viewRadius; }

void phx::World::SetViewRadius(uint32_t radius)
{
	if (radius == m_viewRadius)
		return;

	const uint32_t viewSizePerAxis  = radius * 2 + 1;
	const uint32_t radialDifference = std::abs(static_cast<int>(m_viewRadius) - static_cast<int>(radius));

	m_viewSize = viewSizePerAxis * viewSizePerAxis * viewSizePerAxis;

	// Allocate the memory for the new view, and default fill to nullptr.
	auto temporaryChunks = std::make_unique<Chunk*[]>(m_viewSize);
	std::fill_n(temporaryChunks.get(), m_viewSize, nullptr);

	// If new view is larger: we need to copy the entire current array into the subset of the new array.
	// If the old view is larger: we need to copy a subset of the current array into the whole new array.

	const uint32_t lowerLimit = radialDifference;
	const uint32_t upperLimit = (m_viewRadius > radius ? radius : m_viewRadius) * 2 - radialDifference;

	for (uint32_t i = lowerLimit; i <= upperLimit; ++i)
	{
		for (uint32_t j = lowerLimit; j <= upperLimit; ++j)
		{
			for (uint32_t k = lowerLimit; k <= upperLimit; ++k)
			{
				glm::uvec3 src  = {i, j, k};
				glm::uvec3 dst = {i, j, k};

				if (m_viewRadius > radius)
				{
					dst -= radialDifference;
				}
				else
				{
					src += radialDifference;
				}

				const uint32_t srcIndex = GetIndex3D(src.x, src.y, src.z, m_viewRadius);
				const uint32_t dstIndex = GetIndex3D(dst.x, dst.y, dst.z, radius);

				temporaryChunks[dstIndex] = m_activeView[srcIndex];
			}
		}
	}

	// Now if the new radius is bigger, we need to generate the new chunks.

	m_viewRadius = radius;
	m_activeView.swap(temporaryChunks);
}
