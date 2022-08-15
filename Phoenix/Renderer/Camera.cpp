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
