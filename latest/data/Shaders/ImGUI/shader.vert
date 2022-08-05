#version 450

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec4 inColor;
layout (location = 3) in int inTexID;

layout(set = 1, binding = 0) uniform UniformBufferObjectStatic {
    vec2 ScreenDim;
};

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outColor;
layout (location = 2) out int outTexID;

void main() 
{
	outUV = inUV;
	outColor = inColor;
	outTexID = inTexID;
	gl_Position = vec4(inPos * vec2(2.0f / ScreenDim.x, 2.0f / ScreenDim.y) + vec2(-1.0,-1.0), 0.0, 1.0);
}