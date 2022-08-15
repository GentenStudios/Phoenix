#pragma once

#include <Globals/Globals.hpp>

// Point to cube for collision with off center cubes
// cubePoint is defined as the lower x,y,z position of the cube
bool PointToCube(glm::vec3 point, glm::vec3 cubePoint, float sideLength);