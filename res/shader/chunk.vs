#version 450 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texCoords;
layout (location = 3) in uint i_flags;
layout (location = 4) in uint i_blockType;
layout (location = 5) in int i_blockPosX;
layout (location = 6) in int i_blockPosY;
layout (location = 7) in int i_blockPosZ;
layout (location = 8) in vec2 i_atlasTileCoords;
layout (location = 9) in float i_ambientOcclusion;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_currTime;
uniform int u_isSelectedBlock;
uniform ivec3 u_selectedBlock;
uniform float u_blockBreakProgress;
uniform vec3 u_sunDirection;
uniform float u_ambientLight;
uniform float u_sunLight;
uniform float u_minAmbientOcclusion;
uniform mat4 u_lightSpaceMatrix;

out vec4 v_color;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texCoords;
out float v_diffuse;
out float v_ambientOcclusion;
out vec4 v_lightSpacePosition;
flat out int v_textureLayer;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

void main(void) {
    // water
    if (i_blockType == 2) {
        v_color = vec4(0, 0, 0, 0.7);
    } else {
        v_color = vec4(0, 0, 0, 1);
    }

    v_diffuse = max(dot(i_normal, normalize(u_sunDirection)), 0.0);
    v_ambientOcclusion = mix(
        u_minAmbientOcclusion,
        1.0,
        i_ambientOcclusion);

    v_position = i_position;
    v_normal = i_normal;
    v_texCoords = i_texCoords;
    v_textureLayer = int(i_atlasTileCoords.x);

    ivec3 blockPos = ivec3(i_blockPosX, i_blockPosY, i_blockPosZ);

    vec3 position = i_position;
    if (i_flags == 1) {
        //position += vec3(sin(u_currTime) - sin(u_currTime/2) + sin(u_currTime/4) - sin(u_currTime/8)) * 0.1;
        if (u_isSelectedBlock == 1 && u_selectedBlock == blockPos) {
            float blockBreakProgress = lerp(0, 0.7, u_blockBreakProgress);
            v_color = vec4(0.3 + blockBreakProgress, 0.3, 0.3, 1);
        }
    }

    mat4 mvp = u_projection * u_view;
    gl_Position = mvp * vec4(position, 1.0);
    v_lightSpacePosition = u_lightSpaceMatrix * vec4(position, 1.0);
}
