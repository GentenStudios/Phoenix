#version 450

#extension GL_EXT_nonuniform_qualifier : enable


layout(location = 0) in vec3 outNormal;
layout(location = 1) in vec2 outTexCoord;
layout(location = 2) flat in uint  outMaterial;

layout(location = 0) out vec4 outColor;


void main() 
{
	//vec4 finalColor = vec4(0.0f,1.0f,0.0f,1.0f);
	vec4 finalColor = vec4(outNormal,1.0f);

	outColor = finalColor;
}
