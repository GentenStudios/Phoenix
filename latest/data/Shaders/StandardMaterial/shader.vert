#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include "../_includes/Camera.glsl"

layout(location = 0) in vec3 inPosition;

layout(location = 1) in mat4  modelMatrix;


void main()
{
	gl_Position = CalculateCamera(modelMatrix * vec4(inPosition,1.0f));
}