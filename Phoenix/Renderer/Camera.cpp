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

#include <Renderer/Camera.hpp>

#include <Globals/Globals.hpp>

Camera::Camera(uint32_t width, uint32_t height)
{
	m_direction = glm::vec3(0, 0, -1.0f);
	m_position  = glm::vec3(0, 0, 0);
	SetProjection(width, height);

	m_outOfDateFrustrum = true;
	Update();
}

void Camera::SetProjection(uint32_t width, uint32_t height)
{
	m_projection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.10f, 1000.0f);
	m_projection[1][1] *= -1.0f;

	m_outOfDateFrustrum = true;
}

void Camera::SetWorldPosition(const glm::vec3& position)
{
	m_position          = position;
	m_outOfDateFrustrum = true;
}

void Camera::MoveLocalX(float x)
{
	m_position -= glm::normalize(glm::cross(m_direction, glm::vec3(0, 1.0f, 0))) * x;
	m_outOfDateFrustrum = true;
}

void Camera::MoveWorldY(float y)
{
	m_position.y += y;
	m_outOfDateFrustrum = true;
}

void Camera::MoveLocalZ(float z)
{
	m_position += z * m_direction;
	m_outOfDateFrustrum = true;
}

void Camera::RotatePitch(float x)
{
	m_pitch -= x;

	if (m_pitch > 89.0f)
		m_pitch = 89.0f;
	if (m_pitch < -89.0f)
		m_pitch = -89.0f;

	UpdateCameraRotation();
	m_outOfDateFrustrum = true;
}

void Camera::RotateYaw(float y)
{ 
	m_yaw += y;
	UpdateCameraRotation();
	m_outOfDateFrustrum = true;
}

void Camera::Update()
{
	m_direction   = glm::normalize(m_direction);

	m_view = glm::lookAt(m_position, m_position + m_direction, glm::vec3(0, 1, 0));

	packet.modelToProjection        = m_projection * m_view;
	packet.modelToWorld             = m_view;
	packet.modelToProjectionInverse = glm::inverse(packet.modelToProjection);

	if (m_outOfDateFrustrum)
	{
		UpdateFrustrum(packet.modelToProjection);
		m_outOfDateFrustrum = false;
	}
}

bool Camera::CheckSphereFrustrum(glm::vec3 pos, float radius)
{
	for (auto i = 0; i < 6; i++)
	{
		if ((packet.planes[i].x * pos.x) + (packet.planes[i].y * pos.y) + (packet.planes[i].z * pos.z) + packet.planes[i].w <= -radius)
		{
			return false;
		}
	}
	return true;
}

glm::vec3 Camera::GetPosition() { return m_position; }

glm::vec3 Camera::GetDirection() { return m_direction; }

void Camera::UpdateCameraRotation()
{
	m_direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
	m_direction.y = sin(glm::radians(m_pitch));
	m_direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
}

void Camera::UpdateFrustrum(glm::mat4 matrix)
{
	packet.planes[LEFT].x = matrix[0].w + matrix[0].x;
	packet.planes[LEFT].y = matrix[1].w + matrix[1].x;
	packet.planes[LEFT].z = matrix[2].w + matrix[2].x;
	packet.planes[LEFT].w = matrix[3].w + matrix[3].x;

	packet.planes[RIGHT].x = matrix[0].w - matrix[0].x;
	packet.planes[RIGHT].y = matrix[1].w - matrix[1].x;
	packet.planes[RIGHT].z = matrix[2].w - matrix[2].x;
	packet.planes[RIGHT].w = matrix[3].w - matrix[3].x;

	packet.planes[TOP].x = matrix[0].w - matrix[0].y;
	packet.planes[TOP].y = matrix[1].w - matrix[1].y;
	packet.planes[TOP].z = matrix[2].w - matrix[2].y;
	packet.planes[TOP].w = matrix[3].w - matrix[3].y;

	packet.planes[BOTTOM].x = matrix[0].w + matrix[0].y;
	packet.planes[BOTTOM].y = matrix[1].w + matrix[1].y;
	packet.planes[BOTTOM].z = matrix[2].w + matrix[2].y;
	packet.planes[BOTTOM].w = matrix[3].w + matrix[3].y;

	packet.planes[BACK].x = matrix[0].w + matrix[0].z;
	packet.planes[BACK].y = matrix[1].w + matrix[1].z;
	packet.planes[BACK].z = matrix[2].w + matrix[2].z;
	packet.planes[BACK].w = matrix[3].w + matrix[3].z;

	packet.planes[FRONT].x = matrix[0].w - matrix[0].z;
	packet.planes[FRONT].y = matrix[1].w - matrix[1].z;
	packet.planes[FRONT].z = matrix[2].w - matrix[2].z;
	packet.planes[FRONT].w = matrix[3].w - matrix[3].z;

	for (auto i = 0; i < 6; i++)
	{
		float length = sqrtf(packet.planes[i].x * packet.planes[i].x + packet.planes[i].y * packet.planes[i].y +
		                     packet.planes[i].z * packet.planes[i].z);
		packet.planes[i] /= length;
	}
}

