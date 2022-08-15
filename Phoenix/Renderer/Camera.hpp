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

	struct CameraPacket
	{
		glm::mat4 modelToProjection; // Projection/Position
		glm::mat4 modelToWorld;      // Position
		glm::mat4 modelToProjectionInverse;
		glm::vec4 planes[6];
	} packet;

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
