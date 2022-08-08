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

	void Move(glm::vec3 position);

	void Move(float x, float y, float z);

	void SetPosition(glm::vec3 position);

	void SetPosition(float x, float y, float z);

	void RotateWorldX(float x);

	void RotateWorldY(float y);

	void RotateWorldZ(float z);

	void Rotate(glm::vec3 axis, float angle);

	void Update();

	bool CheckSphereFrustrum(glm::vec3 pos, float radius);

	glm::mat4 GetProjection() { return mProjection; }

private:
	void SetMatrixPosition(glm::vec3 position);

	void      UpdateFrustrum(glm::mat4 matrix);
	glm::mat4 mProjection;
	glm::mat4 mPosition;
	bool      mOutOfDateFrustrum;
};