#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include "../_includes/Camera.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;
layout(location = 3) in int inTextureID;

layout(location = 4) in mat4  modelMatrix;

layout(location = 0) out vec2 outUV;
layout(location = 1) out int outTextureID;
layout(location = 2) out vec3 outNormal;

void main()
{
	gl_Position = CalculateCamera(modelMatrix * vec4(inPosition,1.0f));

	
	outNormal = (modelMatrix * vec4(inNormal,0.0f)).xyz;

	outUV = inUV;
	outTextureID = inTextureID;
}