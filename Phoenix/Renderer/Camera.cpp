#include <Renderer/Camera.hpp>

#include <Globals/Globals.hpp>

Camera::Camera(uint32_t width, uint32_t height)
{
	mDirection = glm::vec3(0,0,-1.0f);
	mPosition  = glm::vec3();
	SetProjection(width, height);

	mOutOfDateFrustrum = true;
	Update();
}

void Camera::SetProjection(uint32_t width, uint32_t height)
{
	mCamera.projection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.10f, 1000.0f);
	mCamera.projection[1][1] *= -1.0f;
	mOutOfDateFrustrum = true;
}

void Camera::SetWorldPosition(glm::vec3 position)
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
	//scale[3][3]    = 1.0f;

	//printf("Pitch:%f Yaw:%f\n", pitch, yaw);

	mDirection   = glm::normalize(mDirection);

	mView = glm::lookAt(mPosition, mPosition + mDirection, glm::vec3(0, 1, 0));

	mCamera.modelToProjection = (mCamera.projection * mView * scale);
	mCamera.modelToWorld = (mView * scale);

	//if (mOutOfDateFrustrum)
	{
		UpdateFrustrum(mCamera.modelToProjection);
		mOutOfDateFrustrum = false;
	}
}

bool Camera::CheckSphereFrustrum(glm::vec3 pos, float radius)
{
	for (auto i = 0; i < 6; i++)
	{
		if ((mCamera.planes[i].x * pos.x) + (mCamera.planes[i].y * pos.y) + (mCamera.planes[i].z * pos.z) + mCamera.planes[i].w <= -radius)
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
	mCamera.planes[LEFT].x = matrix[0].w + matrix[0].x;
	mCamera.planes[LEFT].y = matrix[1].w + matrix[1].x;
	mCamera.planes[LEFT].z = matrix[2].w + matrix[2].x;
	mCamera.planes[LEFT].w = matrix[3].w + matrix[3].x;

	mCamera.planes[RIGHT].x = matrix[0].w - matrix[0].x;
	mCamera.planes[RIGHT].y = matrix[1].w - matrix[1].x;
	mCamera.planes[RIGHT].z = matrix[2].w - matrix[2].x;
	mCamera.planes[RIGHT].w = matrix[3].w - matrix[3].x;

	mCamera.planes[TOP].x = matrix[0].w - matrix[0].y;
	mCamera.planes[TOP].y = matrix[1].w - matrix[1].y;
	mCamera.planes[TOP].z = matrix[2].w - matrix[2].y;
	mCamera.planes[TOP].w = matrix[3].w - matrix[3].y;

	mCamera.planes[BOTTOM].x = matrix[0].w + matrix[0].y;
	mCamera.planes[BOTTOM].y = matrix[1].w + matrix[1].y;
	mCamera.planes[BOTTOM].z = matrix[2].w + matrix[2].y;
	mCamera.planes[BOTTOM].w = matrix[3].w + matrix[3].y;

	mCamera.planes[BACK].x = matrix[0].w + matrix[0].z;
	mCamera.planes[BACK].y = matrix[1].w + matrix[1].z;
	mCamera.planes[BACK].z = matrix[2].w + matrix[2].z;
	mCamera.planes[BACK].w = matrix[3].w + matrix[3].z;

	mCamera.planes[FRONT].x = matrix[0].w - matrix[0].z;
	mCamera.planes[FRONT].y = matrix[1].w - matrix[1].z;
	mCamera.planes[FRONT].z = matrix[2].w - matrix[2].z;
	mCamera.planes[FRONT].w = matrix[3].w - matrix[3].z;

	for (auto i = 0; i < 6; i++)
	{
		float length = sqrtf(mCamera.planes[i].x * mCamera.planes[i].x + mCamera.planes[i].y * mCamera.planes[i].y +
		                     mCamera.planes[i].z * mCamera.planes[i].z);
		mCamera.planes[i] /= length;
	}
}
