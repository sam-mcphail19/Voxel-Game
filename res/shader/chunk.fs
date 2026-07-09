#version 450 core

in vec4 v_color;
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texCoords;
in vec2 v_atlasTileCoords;

out vec4 out_Color;

uniform sampler2D tex;

void main() {
    vec2 tileCount = vec2(textureSize(tex, 0)) / 16.0;
    vec2 localCoords = fract(v_texCoords);
    vec2 atlasCoords = (v_atlasTileCoords + localCoords) / tileCount;

    out_Color = texture(tex, atlasCoords);
    out_Color.a = 0;
    out_Color = out_Color + v_color;
}
