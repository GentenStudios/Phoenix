#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_GOOGLE_include_directive : require

#include "../_includes/Camera.glsl"

vec3 positions[36] = vec3[](
    vec3(-1.0f,-1.0f,-1.0f),
    vec3(-1.0f,-1.0f, 1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3( 1.0f, 1.0f,-1.0f),
    vec3(-1.0f,-1.0f,-1.0f),
    vec3(-1.0f, 1.0f,-1.0f),
    vec3( 1.0f,-1.0f, 1.0f),
    vec3(-1.0f,-1.0f,-1.0f),
    vec3( 1.0f,-1.0f,-1.0f),
    vec3( 1.0f, 1.0f,-1.0f),
    vec3( 1.0f,-1.0f,-1.0f),
    vec3(-1.0f,-1.0f,-1.0f),
    vec3(-1.0f,-1.0f,-1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(-1.0f, 1.0f,-1.0f),
    vec3( 1.0f,-1.0f, 1.0f),
    vec3(-1.0f,-1.0f, 1.0f),
    vec3(-1.0f,-1.0f,-1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3(-1.0f,-1.0f, 1.0f),
    vec3( 1.0f,-1.0f, 1.0f),
    vec3( 1.0f, 1.0f, 1.0f),
    vec3( 1.0f,-1.0f,-1.0f),
    vec3( 1.0f, 1.0f,-1.0f),
    vec3( 1.0f,-1.0f,-1.0f),
    vec3( 1.0f, 1.0f, 1.0f),
    vec3( 1.0f,-1.0f, 1.0f),
    vec3( 1.0f, 1.0f, 1.0f),
    vec3( 1.0f, 1.0f,-1.0f),
    vec3(-1.0f, 1.0f,-1.0f),
    vec3( 1.0f, 1.0f, 1.0f),
    vec3(-1.0f, 1.0f,-1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3( 1.0f, 1.0f, 1.0f),
    vec3(-1.0f, 1.0f, 1.0f),
    vec3( 1.0f,-1.0f, 1.0f)
);

layout(location = 0) out vec3 eyeDirection;

void main()
{
    eyeDirection = positions[gl_VertexIndex];

	gl_Position = (camera.projection * mat4(mat3(camera.modelToWorld)) * vec4(positions[gl_VertexIndex], 1.f)).xyww;
}
