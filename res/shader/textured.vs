#version 450 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texCoords;
layout (location = 3) in uint i_flags;
layout (location = 4) in uint i_blockType;
layout (location = 5) in int i_blockPosX;
layout (location = 6) in int i_blockPosY;
layout (location = 7) in int i_blockPosZ;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_currTime;
uniform int u_isSelectedBlock;
uniform ivec3 u_selectedBlock;

out vec4 v_color;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texCoords;

void main(void) {
    v_color = vec4(0);
    v_position = i_position;
    v_normal = i_normal;
    v_texCoords = i_texCoords;

    vec3 position = i_position;
    if (i_flags == 1) {
        //position += vec3(sin(u_currTime) - sin(u_currTime/2) + sin(u_currTime/4) - sin(u_currTime/8)) * 0.1;
        if (u_isSelectedBlock == 1 && u_selectedBlock == ivec3(i_blockPosX, i_blockPosY, i_blockPosZ)) {
            v_color = vec4(0.3, 0.3, 0.3, 1);
        }
    }

    mat4 mvp = u_projection * u_view * u_model;
    gl_Position = mvp * vec4(position, 1.0);
}