#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include "../_includes/Camera.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in uint  inMaterial;

layout(location = 4) in mat4  modelMatrix;


layout(location = 0) out vec3 outNormal;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) flat out uint  outMaterial;

void main()
{
	gl_Position = CalculateCamera(modelMatrix * vec4(inPosition,1.0f));
	outNormal = inNormal;
	outTexCoord = inTexCoord;
	outMaterial = inMaterial;
}