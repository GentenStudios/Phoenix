#pragma once

#include <Globals/Globals.hpp>

#include <map>
#include <memory>

class Camera
{
public:
	enum side
	{
		LEFT   = 0,
		RIGHT  = 1,
		TOP    = 2,
		BOTTOM = 3,
		BACK   = 4,
		FRONT  = 5
	};
	struct CameraPacket
	{
		glm::mat4 ProPos; // Projection/Position
		glm::vec4 planes[6];
	} mCamera;

	Camera(uint32_t width, uint32_t height);

	void SetProjection(uint32_t width, uint32_t height);

	void SetWorldPosition(glm::vec3 position);

	void MoveLocalX(float x);

	void MoveWorldY(float y);

	void MoveLocalZ(float z);

	void RotatePitch(float x);

	void RotateYaw(float y);

	void Update();

	bool CheckSphereFrustrum(glm::vec3 pos, float radius);

	glm::mat4 GetProjection() { return mProjection; }

private:

	void UpdateCameraRotation();

	void      UpdateFrustrum(glm::mat4 matrix);

	
	float     mPitch = 0.0f;
	float     mYaw   = 0.0f;

	glm::mat4 mProjection;
	glm::mat4 mView;

	glm::vec3 mPosition;
	glm::vec3 mDirection;

	bool      mOutOfDateFrustrum;
};