#include <Renderer/Camera.hpp>

#include <Globals/Globals.hpp>

Camera::Camera(uint32_t width, uint32_t height)
{
	SetMatrixPosition(glm::vec3());
	SetProjection(width, height);

	Update();
}

void Camera::SetProjection(uint32_t width, uint32_t height)
{
	mProjection = glm::perspective(glm::radians(45.0f), (float) width / (float) height, 0.10f, 1000.0f);
	mProjection[1][1] *= -1.0f;
}

void Camera::Move(glm::vec3 position) { mPosition = glm::translate(mPosition, position); }

void Camera::Move(float x, float y, float z){ Move(glm::vec3(x, y, z)); }

void Camera::SetPosition(glm::vec3 position) { SetMatrixPosition(-position); }

void Camera::SetPosition(float x, float y, float z) { SetPosition(glm::vec3(x, y, z)); }

void Camera::RotateWorldX(float x) { Rotate(glm::vec3(1.0f, 0.0f, 0.0f), x); }

void Camera::RotateWorldY(float y) { Rotate(glm::vec3(1.0f, 0.0f, 0.0f), y); }

void Camera::RotateWorldZ(float z) { Rotate(glm::vec3(1.0f, 0.0f, 0.0f), z); }

void Camera::Rotate(glm::vec3 axis, float angle)
{ 
	mPosition = glm::rotate(mPosition, glm::radians(angle), axis); }

void Camera::Update()
{
	glm::mat4 scale(1.0f);
	scale[3][3]    = 1.0f;

	mCamera.ProPos = (mProjection * mPosition * scale);
	UpdateFrustrum(mCamera.ProPos);
	mOutOfDateFrustrum = false;
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

void Camera::SetMatrixPosition(glm::vec3 position)
{
	// mPosition = glm::lookAt( glm::vec3( 0, 0, 1 ), glm::vec3( 0, 0, 0 ), glm::vec3( 0, 1, 0 ) );
	mPosition = glm::translate(glm::mat4(1.0f), position);
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
