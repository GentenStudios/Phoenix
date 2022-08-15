#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include "../_includes/Camera.glsl"

vec2 screenPos[6] = vec2[](
    vec2(-1, -1), 
    vec2( 1, -1), 
    vec2(-1,  1), 
    vec2(-1,  1),
    vec2( 1, -1),
    vec2( 1,  1)
);

layout(location = 0) smooth out vec3 texCoord;

void main()
{
    vec4 translatedPosition = camera.modelToProjectionInverse * vec4(screenPos[gl_VertexIndex], 1.f, 1.f);
    texCoord = normalize(translatedPosition.xyz / translatedPosition.w);

    gl_Position = vec4(screenPos[gl_VertexIndex], 1.f, 1.f);
}
