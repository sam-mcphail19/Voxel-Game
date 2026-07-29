#version 450 core

in vec3 v_position;
in vec3 v_surfaceNormal;
in vec2 v_texCoords;
in vec4 v_lightSpacePosition;
in float v_shoreProximity;
in float v_wavePhaseA;
in float v_wavePhaseB;
in float v_waveStrength;
in vec2 v_waveDerivativeA;
in vec2 v_waveDerivativeB;
flat in int v_isTopFace;
flat in int v_textureLayer;

out vec4 out_Color;

uniform sampler2DArray tex;
uniform sampler2DShadow u_shadowMap;
uniform float u_currTime;
uniform vec3 u_sunDirection;
uniform float u_ambientLight;
uniform float u_sunLight;
uniform vec3 u_cameraPosition;
uniform float u_fogStart;
uniform float u_fogEnd;
uniform float u_hazeStrength;
uniform float u_fogBaseHeight;

// ---------------------------------------------------------------------------
// Water appearance tuning
// ---------------------------------------------------------------------------
// Texture drift is measured in texture tiles per second.
const vec2 TEXTURE_DRIFT_SPEED = vec2(0.018, -0.011);

const vec3 SHALLOW_WATER_COLOR = vec3(0.12, 0.53, 0.66);
const vec3 DEEP_WATER_COLOR = vec3(0.035, 0.20, 0.40);
const vec3 SKY_REFLECTION_COLOR = vec3(0.42, 0.62, 0.86);
const float TEXTURE_COLOR_VARIATION = 0.55;
const float SKY_REFLECTION_STRENGTH = 0.68;
const float MIN_WATER_LIGHT = 0.72;

const float FRESNEL_POWER = 4.0;
const float MIN_WATER_ALPHA = 0.58;
const float MAX_WATER_ALPHA = 0.82;

const vec3 SHORE_RIPPLE_COLOR = vec3(0.65, 0.86, 0.92);
const float SHORE_RIPPLE_SPEED = 2.2;
const float SHORE_RIPPLE_SHARPNESS = 5.0;
const float SHORE_RIPPLE_STRENGTH = 0.24;
const float SHORE_RIPPLE_FADE_START = 0.05;
const float SHORE_RIPPLE_FADE_END = 0.90;
const float SHORE_RIPPLE_MIN_UP_NORMAL = 0.70;
const float SHORE_RIPPLE_FULL_UP_NORMAL = 0.95;
const float SHORE_RIPPLE_BAND_COUNT = 3.0;
const vec2 SHORE_RIPPLE_DISTORTION_DIRECTION = vec2(0.83, 0.56);
const float SHORE_RIPPLE_DISTORTION_FREQUENCY = 0.55;
const float SHORE_RIPPLE_DISTORTION_STRENGTH = 0.35;
const float TAU = 6.28318530718;

const vec3 SUN_GLINT_COLOR = vec3(1.0, 0.88, 0.62);
const float SUN_GLINT_SHARPNESS = 96.0;
const float SUN_GLINT_STRENGTH = 0.75;

// ---------------------------------------------------------------------------
// Shadow and atmosphere tuning
// ---------------------------------------------------------------------------
const float SHADOW_BIAS_AT_GRAZING_ANGLE = 0.0020;
const float SHADOW_BIAS_FACING_SUN = 0.00055;
const float SHADOW_FADE_START = 220.0;
const float SHADOW_FADE_END = 300.0;

const float HORIZON_HAZE_INNER_SLOPE = 0.05;
const float HORIZON_HAZE_OUTER_SLOPE = 0.55;
const float HAZE_HEIGHT_FALLOFF = 0.006;
const float HAZE_DISTANCE_START_FACTOR = 0.45;
const float SKY_GRADIENT_BOTTOM = -0.18;
const float SKY_GRADIENT_TOP = 0.55;
const vec3 HORIZON_FOG_COLOR = vec3(0.52, 0.66, 0.86);
const vec3 ZENITH_FOG_COLOR = vec3(0.15, 0.28, 0.58);

// Fixed numerical and coordinate-system constants.
const float NORMALIZED_DEPTH_MIN = 0.0;
const float NORMALIZED_DEPTH_MAX = 1.0;
const float CLIP_TO_TEXTURE_SCALE = 0.5;
const float CLIP_TO_TEXTURE_OFFSET = 0.5;
const float SAFE_NORMALIZE_EPSILON = 0.0001;

vec2 waterTextureCoordinates(vec3 normal) {
    if (abs(normal.y) >= SHORE_RIPPLE_MIN_UP_NORMAL) {
        return v_position.xz;
    }
    if (abs(normal.x) > abs(normal.z)) {
        return v_position.zy;
    }
    return v_position.xy;
}

vec3 calculateWaterNormal() {
    if (v_isTopFace == 0) {
        return normalize(v_surfaceNormal);
    }

    vec2 slope = cos(v_wavePhaseA) * v_waveDerivativeA
        + cos(v_wavePhaseB) * v_waveDerivativeB;
    vec3 waveNormal = normalize(vec3(-slope.x, 1.0, -slope.y));
    return normalize(mix(
        vec3(0.0, 1.0, 0.0),
        waveNormal,
        v_waveStrength
    ));
}

float sampleSunShadow(float diffuse) {
    vec3 projected = v_lightSpacePosition.xyz / v_lightSpacePosition.w;
    projected = projected * CLIP_TO_TEXTURE_SCALE + CLIP_TO_TEXTURE_OFFSET;

    if (
        projected.z <= NORMALIZED_DEPTH_MIN
        || projected.z >= NORMALIZED_DEPTH_MAX
        || projected.x <= NORMALIZED_DEPTH_MIN
        || projected.x >= NORMALIZED_DEPTH_MAX
        || projected.y <= NORMALIZED_DEPTH_MIN
        || projected.y >= NORMALIZED_DEPTH_MAX
    ) {
        return 0.0;
    }

    float normalBias = mix(
        SHADOW_BIAS_AT_GRAZING_ANGLE,
        SHADOW_BIAS_FACING_SUN,
        diffuse
    );
    float shadow = 1.0 - texture(
        u_shadowMap,
        vec3(projected.xy, projected.z - normalBias)
    );
    float cameraDistance = length(v_position.xz - u_cameraPosition.xz);
    float shadowCoverage = 1.0 - smoothstep(
        SHADOW_FADE_START,
        SHADOW_FADE_END,
        cameraDistance
    );

    return shadow * shadowCoverage;
}

float calculateFogAmount(
    vec3 outwardViewDirection,
    float viewDistance
) {
    float distanceFog = smoothstep(u_fogStart, u_fogEnd, viewDistance);
    float horizonAmount = 1.0 - smoothstep(
        HORIZON_HAZE_INNER_SLOPE,
        HORIZON_HAZE_OUTER_SLOPE,
        abs(outwardViewDirection.y)
    );
    float heightAboveFog = max(v_position.y - u_fogBaseHeight, 0.0);
    float heightFalloff = exp(-heightAboveFog * HAZE_HEIGHT_FALLOFF);
    float heightHaze = horizonAmount * heightFalloff
        * u_hazeStrength
        * smoothstep(
            u_fogStart * HAZE_DISTANCE_START_FACTOR,
            u_fogEnd,
            viewDistance
        );

    return clamp(max(distanceFog, heightHaze), 0.0, 1.0);
}

vec3 calculateFogColor(vec3 outwardViewDirection) {
    float skyHeight = smoothstep(
        SKY_GRADIENT_BOTTOM,
        SKY_GRADIENT_TOP,
        outwardViewDirection.y
    );

    return mix(HORIZON_FOG_COLOR, ZENITH_FOG_COLOR, skyHeight);
}

float calculateShoreRipple(vec3 normal) {
    float shoreMask = smoothstep(
        SHORE_RIPPLE_FADE_START,
        SHORE_RIPPLE_FADE_END,
        v_shoreProximity
    );
    float topFaceMask = smoothstep(
        SHORE_RIPPLE_MIN_UP_NORMAL,
        SHORE_RIPPLE_FULL_UP_NORMAL,
        normal.y
    );
    float distortion = sin(
        dot(v_position.xz, SHORE_RIPPLE_DISTORTION_DIRECTION)
            * SHORE_RIPPLE_DISTORTION_FREQUENCY
    ) * SHORE_RIPPLE_DISTORTION_STRENGTH;
    float phase = (1.0 - v_shoreProximity) * SHORE_RIPPLE_BAND_COUNT * TAU
        + distortion
        - u_currTime * SHORE_RIPPLE_SPEED;
    float rippleBand = pow(
        0.5 + 0.5 * sin(phase),
        SHORE_RIPPLE_SHARPNESS
    );
    return rippleBand * shoreMask * topFaceMask;
}

void main() {
    vec3 normal = calculateWaterNormal();
    vec3 viewDirection = normalize(u_cameraPosition - v_position);
    vec3 sunDirection = normalize(u_sunDirection);

    vec2 continuousUv = waterTextureCoordinates(normal);
    vec2 animatedUv = continuousUv + u_currTime * TEXTURE_DRIFT_SPEED;
    vec4 waterTexture = textureGrad(
        tex,
        vec3(fract(animatedUv), float(v_textureLayer)),
        dFdx(continuousUv),
        dFdy(continuousUv)
    );

    float fresnel = pow(
        1.0 - clamp(dot(normal, viewDirection), 0.0, 1.0),
        FRESNEL_POWER
    );
    float diffuse = max(dot(normal, sunDirection), 0.0);
    float shadow = sampleSunShadow(diffuse);
    float light = min(
        u_ambientLight + diffuse * u_sunLight * (1.0 - shadow),
        1.0
    );

    vec3 baseColor = mix(
        DEEP_WATER_COLOR,
        SHALLOW_WATER_COLOR,
        waterTexture.r * TEXTURE_COLOR_VARIATION
    );
    baseColor = mix(
        baseColor,
        SKY_REFLECTION_COLOR,
        fresnel * SKY_REFLECTION_STRENGTH
    );
    baseColor *= mix(MIN_WATER_LIGHT, 1.0, light);
    float shoreRipple = calculateShoreRipple(normal);
    baseColor = mix(
        baseColor,
        SHORE_RIPPLE_COLOR,
        shoreRipple * SHORE_RIPPLE_STRENGTH
    );

    vec3 reflectedSun = reflect(-sunDirection, normal);
    float sunGlint = pow(
        max(dot(reflectedSun, viewDirection), 0.0),
        SUN_GLINT_SHARPNESS
    );
    baseColor += SUN_GLINT_COLOR * sunGlint * (1.0 - shadow) * SUN_GLINT_STRENGTH;

    vec3 cameraToFragment = v_position - u_cameraPosition;
    float viewDistance = length(cameraToFragment);
    vec3 outwardViewDirection = cameraToFragment / max(viewDistance, SAFE_NORMALIZE_EPSILON);
    float fogAmount = calculateFogAmount(
        outwardViewDirection,
        viewDistance
    );
    vec3 fogColor = calculateFogColor(outwardViewDirection);

    out_Color.rgb = mix(baseColor, fogColor, fogAmount);
    float waterAlpha = mix(
        MIN_WATER_ALPHA,
        MAX_WATER_ALPHA,
        fresnel
    );
    out_Color.a = mix(waterAlpha, 1.0, fogAmount);
}
