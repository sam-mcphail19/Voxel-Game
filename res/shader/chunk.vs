#version 450 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texCoords;
layout (location = 3) in uint i_flags;
layout (location = 4) in uint i_blockType;
layout (location = 5) in int i_blockPosX;
layout (location = 6) in int i_blockPosY;
layout (location = 7) in int i_blockPosZ;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform float u_currTime;
uniform int u_isSelectedBlock;
uniform ivec3 u_selectedBlock;
uniform float u_blockBreakProgress;

out vec4 v_color;
out vec3 v_position;
out vec3 v_normal;
out vec2 v_texCoords;

float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

int floorDiv(int a, int b) {
    int result = a / b;
	// If a and b have opposite signs and there's a remainder, subtract 1 from the result
	if ((a % b != 0) && ((a < 0) != (b < 0))) {
		result--;
	}
	return result;
}

ivec3 getChunkOrigin(ivec3 blockPos) {
    // TODO: Pass these as uniforms (at start up, not every frame, because they dont change)
    int x = floorDiv(blockPos.x, 16);
	int y = floorDiv(blockPos.y, 128);
	int z = floorDiv(blockPos.z, 16);

	if (y < 0) {
        y = 0;
    }

	return ivec3(x * 16, y * 128, z * 16);
}

void main(void) {
    // water
    if (i_blockType == 2) {
        v_color = vec4(0, 0, 0, 0.7);
    } else {
        v_color = vec4(0, 0, 0, 1);
    }

    // TODO: Real lighting
    if (i_normal == vec3(1, 0, 0) || i_normal == vec3(-1, 0, 0)) {
        v_color = v_color - vec4(vec3(0.1), 0);
    } else if (i_normal == vec3(0, 0, 1) || i_normal == vec3(0, 0, -1)) {
        v_color = v_color - vec4(vec3(0.05), 0);
    }

    v_position = i_position;
    v_normal = i_normal;
    v_texCoords = i_texCoords;

    ivec3 blockPos = ivec3(i_blockPosX, i_blockPosY, i_blockPosZ);

    // Need to get chunk origin from blockPos and add it to i_position
    vec3 position = i_position + getChunkOrigin(blockPos);
    if (i_flags == 1) {
        //position += vec3(sin(u_currTime) - sin(u_currTime/2) + sin(u_currTime/4) - sin(u_currTime/8)) * 0.1;
        if (u_isSelectedBlock == 1 && u_selectedBlock == blockPos) {
            float blockBreakProgress = lerp(0, 0.7, u_blockBreakProgress);
            v_color = vec4(0.3 + blockBreakProgress, 0.3, 0.3, 1);
        }
    }

    mat4 mvp = u_projection * u_view;
    gl_Position = mvp * vec4(position, 1.0);
}
