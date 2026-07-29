#version 450 core

layout (location = 0) in vec3 i_position;

uniform mat4 u_lightSpaceMatrix;

void main() {
    gl_Position = u_lightSpaceMatrix * vec4(i_position, 1.0);
}
