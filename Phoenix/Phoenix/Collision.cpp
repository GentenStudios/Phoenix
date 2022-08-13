#include <Phoenix/Collision.hpp>

bool PointToCube(glm::vec3 point, glm::vec3 cubePoint, float sideLength)
{
	// Early out
	if (point.x <= cubePoint.x || point.y <= cubePoint.y || point.z <= cubePoint.z || 
		point.x > cubePoint.x + sideLength || point.y > cubePoint.y + sideLength || point.z > cubePoint.z + sideLength)
	{
		return false;
	}
	return true;
}
