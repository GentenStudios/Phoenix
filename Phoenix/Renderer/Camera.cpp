#include <Renderer/Camera.hpp>

#include <Globals/Globals.hpp>

Camera::Camera(uint32_t width, uint32_t height)
{
	mDirection = glm::vec3(0, 0, -1.0f);
	mPosition  = glm::vec3(0, 0, 0);
	SetProjection(width, height);

	mOutOfDateFrustrum = true;
	Update();
}

void Camera::SetProjection(uint32_t width, uint32_t height)
{
	mProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.10f, 1000.0f);
	mProjection[1][1] *= -1.0f;

	mOutOfDateFrustrum = true;
}

void Camera::SetWorldPosition(const glm::vec3& position)
{
	mPosition          = position;
	mOutOfDateFrustrum = true;
}

void Camera::MoveLocalX(float x)
{
	mPosition -= glm::normalize(glm::cross(mDirection, glm::vec3(0, 1.0f, 0))) * x;
	mOutOfDateFrustrum = true;
}

void Camera::MoveWorldY(float y)
{
	mPosition.y += y;
	mOutOfDateFrustrum = true;
}

void Camera::MoveLocalZ(float z)
{
	mPosition += z * mDirection;
	mOutOfDateFrustrum = true;
}

void Camera::RotatePitch(float x)
{
	mPitch -= x;

	if (mPitch > 89.0f)
		mPitch = 89.0f;
	if (mPitch < -89.0f)
		mPitch = -89.0f;

	UpdateCameraRotation();
	mOutOfDateFrustrum = true;
}

void Camera::RotateYaw(float y)
{ 
	mYaw += y;
	UpdateCameraRotation();
	mOutOfDateFrustrum = true;
}

void Camera::Update()
{
	glm::mat4 scale(1.0f);

	mDirection   = glm::normalize(mDirection);

	mView = glm::lookAt(mPosition, mPosition + mDirection, glm::vec3(0, 1, 0));

	packet.modelToProjection = (mProjection * mView);
	packet.modelToWorld      = mView * scale;
	packet.projection        = mProjection;

	if (mOutOfDateFrustrum)
	{
		UpdateFrustrum(packet.modelToProjection);
		mOutOfDateFrustrum = false;
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

glm::vec3 Camera::GetPosition() { return mPosition; }

glm::vec3 Camera::GetDirection() { return mDirection; }

void Camera::UpdateCameraRotation()
{
	mDirection.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
	mDirection.y = sin(glm::radians(mPitch));
	mDirection.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
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
