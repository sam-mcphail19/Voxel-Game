#version 450 core

layout (location = 0) in vec3 i_position;
layout (location = 1) in vec3 i_normal;
layout (location = 2) in vec2 i_texCoords;
layout (location = 8) in vec2 i_atlasTileCoords;
layout (location = 9) in float i_waterData;

uniform mat4 u_view;
uniform mat4 u_projection;
uniform mat4 u_lightSpaceMatrix;
uniform float u_currTime;
uniform vec3 u_cameraPosition;

out vec3 v_position;
out vec3 v_surfaceNormal;
out vec2 v_texCoords;
out vec4 v_lightSpacePosition;
out float v_shoreProximity;
out float v_wavePhaseA;
out float v_wavePhaseB;
out float v_waveStrength;
out vec2 v_waveDerivativeA;
out vec2 v_waveDerivativeB;
flat out int v_isTopFace;
flat out int v_textureLayer;

// ---------------------------------------------------------------------------
// Water geometry tuning
// ---------------------------------------------------------------------------
// Frequencies are radians per world block. Speeds are radians per second.
// Amplitudes are measured in world blocks.
const vec2 WAVE_A_SPATIAL_FREQUENCY = vec2(0.115, 0.071);
const vec2 WAVE_B_SPATIAL_FREQUENCY = vec2(-0.064, 0.137);
const float WAVE_A_SPEED = 0.85;
const float WAVE_B_SPEED = -0.62;
const float WAVE_A_AMPLITUDE = 0.2;
const float WAVE_B_AMPLITUDE = 0.1;
// Calm water sits below the top of its containing voxel, leaving room for
// crests without making the average surface spill over shoreline blocks.
const float CALM_SURFACE_DEPTH = 0.12;
const float FACE_UP_THRESHOLD = 0.70;
const float FACE_DOWN_THRESHOLD = -0.70;
const float SIDE_TOP_UV_THRESHOLD = 0.50;
const float SHORELINE_SKIRT_INSET = 0.003;
const float SHORE_WAVE_DAMP_START = 0.15;
const float SHORE_WAVE_DAMP_END = 0.85;
// Shore waves retain a small amount of motion. At this strength their
// maximum crest remains below the containing voxel's top edge.
const float MIN_SHORE_WAVE_STRENGTH = 0.35;
// Geometry becomes flat before terrain switches to half-resolution chunks.
// Shading animation remains active beyond this range.
const float GEOMETRY_WAVE_FADE_START = 190.0;
const float GEOMETRY_WAVE_FADE_END = 235.0;

float wavePhase(
    vec2 worldPosition,
    vec2 spatialFrequency,
    float speed
) {
    return dot(worldPosition, spatialFrequency) + u_currTime * speed;
}

float waveHeight(float phaseA, float phaseB) {
    return sin(phaseA) * WAVE_A_AMPLITUDE + sin(phaseB) * WAVE_B_AMPLITUDE;
}

void main() {
    float phaseA = wavePhase(i_position.xz, WAVE_A_SPATIAL_FREQUENCY, WAVE_A_SPEED);
    float phaseB = wavePhase(i_position.xz, WAVE_B_SPATIAL_FREQUENCY, WAVE_B_SPEED);

    bool isTopFace = i_normal.y >= FACE_UP_THRESHOLD;
    bool isBottomFace = i_normal.y <= FACE_DOWN_THRESHOLD;
    bool isSideFace = !isTopFace && !isBottomFace;
    bool isTopOfSideFace = isSideFace && i_texCoords.y > SIDE_TOP_UV_THRESHOLD;
    float shoreProximity = isBottomFace ? 0.0 : i_waterData;
    float shoreWaveStrength = mix(
        MIN_SHORE_WAVE_STRENGTH,
        1.0,
        1.0 - smoothstep(
            SHORE_WAVE_DAMP_START,
            SHORE_WAVE_DAMP_END,
            shoreProximity
        )
    );
    float cameraDistance = length(i_position.xz - u_cameraPosition.xz);
    float distanceWaveStrength = 1.0 - smoothstep(
        GEOMETRY_WAVE_FADE_START,
        GEOMETRY_WAVE_FADE_END,
        cameraDistance
    );
    float waveStrength = (isTopFace || isTopOfSideFace)
        ? shoreWaveStrength * distanceWaveStrength
        : 0.0;
    float animatedSurfaceOffset =
        waveHeight(phaseA, phaseB) * waveStrength - CALM_SURFACE_DEPTH;

    vec3 displacedPosition = i_position;
    if (isTopFace) {
        displacedPosition.y += animatedSurfaceOffset;
    } else if (isTopOfSideFace) {
        displacedPosition.y += animatedSurfaceOffset;
    }
    if (isSideFace) {
        displacedPosition -= normalize(i_normal) * SHORELINE_SKIRT_INSET;
    }

    v_position = displacedPosition;
    v_surfaceNormal = normalize(i_normal);
    v_texCoords = i_texCoords;
    v_shoreProximity = isTopFace ? shoreProximity : 0.0;
    v_wavePhaseA = phaseA;
    v_wavePhaseB = phaseB;
    v_waveStrength = waveStrength;
    v_waveDerivativeA = WAVE_A_AMPLITUDE * WAVE_A_SPATIAL_FREQUENCY;
    v_waveDerivativeB = WAVE_B_AMPLITUDE * WAVE_B_SPATIAL_FREQUENCY;
    v_isTopFace = isTopFace ? 1 : 0;
    v_textureLayer = int(i_atlasTileCoords.x);
    v_lightSpacePosition = u_lightSpaceMatrix * vec4(displacedPosition, 1.0);
    gl_Position = u_projection * u_view * vec4(displacedPosition, 1.0);
}
