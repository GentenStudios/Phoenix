#version 460

#extension GL_EXT_nonuniform_qualifier : enable

layout (set = 1, binding = 0) uniform sampler2D[32] textures;

layout(location = 0) in vec2 inUV;
layout(location = 1) flat in int inTextureID;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outColor;


void main() 
{
	vec4 diffuseColor = texture(textures[inTextureID], inUV);

	vec3 fakeLight =normalize(vec3(1,2,3));

	vec3 normal = normalize(inNormal);

	float dc = dot(normal,fakeLight);

	vec3 color;

	float ambient = 0.4f;

	color = diffuseColor.xyz * dc + diffuseColor.xyz * ambient;

	outColor = vec4(color,1.0f);
	//outColor = vec4(inNormal,1.0f);
}
