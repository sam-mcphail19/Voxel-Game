#version 450 core

in vec4 v_color;
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texCoords;

out vec4 out_Color;

uniform sampler2D tex;

void main() {
    out_Color = texture(tex, v_texCoords);
    out_Color.a = 0;
    out_Color = out_Color + v_color;
}