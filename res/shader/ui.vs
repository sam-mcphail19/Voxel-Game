#version 450 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texCoords;
layout (location = 3) in uint i_flags;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_currTime;

out vec4 v_color;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texCoords;

void main(void) {
    v_color = vec4(0);
    v_position = i_position;
    v_normal = i_normal;
    v_texCoords = i_texCoords;

    mat4 mvp = u_projection * u_view * u_model;
    gl_Position = mvp * vec4(i_position, 1.0);
}