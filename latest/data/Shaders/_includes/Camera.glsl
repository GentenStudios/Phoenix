struct Camera
{
    mat4 modelToProjection;
    mat4 modelToWorld;
	vec4 frustumPlanes[6];
};

layout (binding=0, set = 0) readonly uniform CameraBuffer {Camera camera; };


vec4 CalculateCamera(vec4 position)
{
	return camera.modelToProjection * position;
}