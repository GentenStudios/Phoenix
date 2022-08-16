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

#include <Globals/Globals.hpp>

#include <map>
#include <memory>

class Camera
{
public:
	enum Side
	{
		LEFT   = 0,
		RIGHT  = 1,
		TOP    = 2,
		BOTTOM = 3,
		BACK   = 4,
		FRONT  = 5
	};

public:
	Camera(uint32_t width, uint32_t height);

	void SetProjection(uint32_t width, uint32_t height);
	void SetWorldPosition(const glm::vec3& position);

	void MoveLocalX(float x);
	void MoveWorldY(float y);
	void MoveLocalZ(float z);

	void RotatePitch(float x);
	void RotateYaw(float y);

	void Update();

	bool CheckSphereFrustrum(glm::vec3 pos, float radius);

	glm::vec3 GetPosition();
	glm::vec3 GetDirection();
	glm::mat4 GetProjection() { return m_projection; }

	struct CameraPacket
	{
		glm::mat4 modelToProjection; // Projection/Position
		glm::mat4 modelToWorld;      // Position
		glm::mat4 modelToProjectionInverse;
		glm::vec4 planes[6];
	} packet;

private:
	void UpdateCameraRotation();
	void UpdateFrustrum(glm::mat4 matrix);

private:
	float m_pitch = 0.0f;
	float m_yaw   = 0.0f;

	glm::mat4 m_projection;
	glm::mat4 m_view;

	glm::vec3 m_position;
	glm::vec3 m_direction;

	bool m_outOfDateFrustrum;
};
