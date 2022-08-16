#version 460

#extension GL_EXT_nonuniform_qualifier : enable

layout(set = 1, binding = 0) uniform samplerCube skyboxTexture;

layout(location = 0) smooth in vec3 eyeDirection;
layout(location = 0) out vec4 outColor;

void main() 
{
	outColor = texture(skyboxTexture, eyeDirection);
}
