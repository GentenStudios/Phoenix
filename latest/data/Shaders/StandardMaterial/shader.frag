#version 450

#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) out vec4 outColor;


void main() 
{
	vec4 finalColor = vec4(0.0f,1.0f,0.0f,1.0f);

	outColor = finalColor;
}
