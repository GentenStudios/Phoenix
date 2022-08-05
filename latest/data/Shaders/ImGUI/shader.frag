#version 460

#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 0, binding = 0) uniform sampler2D[100] textures;

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec4 inColor;
layout (location = 2) flat in int inTexID;

layout (location = 0) out vec4 outColor;

void main() 
{
	outColor = inColor;
	if(inTexID > 0)
	{
		outColor *= texture(textures[inTexID - 1], inUV);
	}
}