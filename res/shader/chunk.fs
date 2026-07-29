#version 450 core

in vec4 v_color;
in vec3 v_position;
in vec3 v_normal;
in vec2 v_texCoords;
in float v_diffuse;
in float v_ambientOcclusion;
in vec4 v_lightSpacePosition;
flat in int v_textureLayer;

out vec4 out_Color;

uniform sampler2DArray tex;
uniform sampler2DShadow u_shadowMap;
uniform float u_ambientLight;
uniform float u_sunLight;
uniform vec3 u_cameraPosition;
uniform float u_fogStart;
uniform float u_fogEnd;
uniform float u_hazeStrength;
uniform float u_fogBaseHeight;

float sampleSunShadow() {
    vec3 projected = v_lightSpacePosition.xyz / v_lightSpacePosition.w;
    projected = projected * 0.5 + 0.5;
    if (projected.z <= 0.0 || projected.z >= 1.0
        || projected.x <= 0.0 || projected.x >= 1.0
        || projected.y <= 0.0 || projected.y >= 1.0) {
        return 0.0;
    }

    // VSM compares a continuous depth distribution and needs far less bias
    // than the previous binary comparison map. The old values displaced
    // receivers by a substantial fraction of a block and erased shadows.
    float normalBias = mix(0.0020, 0.00055, v_diffuse);
    float shadow = 1.0 - texture(
        u_shadowMap,
        vec3(projected.xy, projected.z - normalBias));

    float cameraDistance = length(v_position.xz - u_cameraPosition.xz);
    float coverageFade = 1.0 - smoothstep(
        220.0,
        300.0,
        cameraDistance);
    return shadow * coverageFade;
}

void main() {
    vec2 localCoords = fract(v_texCoords);

    // Compute gradients before wrapping the greedy-mesh UVs. Derivatives of
    // fract() spike at tile boundaries and otherwise select overly blurry mips.
    out_Color = textureGrad(
        tex,
        vec3(localCoords, float(v_textureLayer)),
        dFdx(v_texCoords),
        dFdy(v_texCoords));
    float shadow = sampleSunShadow();
    float light = min(
        u_ambientLight + v_diffuse * u_sunLight * (1.0 - shadow),
        1.0) * v_ambientOcclusion;
    out_Color.rgb *= light;
    out_Color.rgb += v_color.rgb;

    vec3 viewVector = v_position - u_cameraPosition;
    float viewDistance = length(viewVector);
    vec3 viewDirection = viewVector / max(viewDistance, 0.0001);
    float distanceFog = smoothstep(
        u_fogStart,
        u_fogEnd,
        viewDistance);

    // Extra near-horizon haze adds aerial perspective in valleys without
    // washing out nearby elevated terrain.
    float horizonAmount = 1.0 - smoothstep(
        0.05,
        0.55,
        abs(viewDirection.y));
    float heightFalloff = exp(
        -max(v_position.y - u_fogBaseHeight, 0.0) * 0.006);
    float heightHaze = horizonAmount * heightFalloff
        * u_hazeStrength
        * smoothstep(u_fogStart * 0.45, u_fogEnd, viewDistance);
    float fogAmount = clamp(max(distanceFog, heightHaze), 0.0, 1.0);

    float skyHeight = smoothstep(-0.18, 0.55, viewDirection.y);
    vec3 horizonColor = vec3(0.52, 0.66, 0.86);
    vec3 zenithColor = vec3(0.15, 0.28, 0.58);
    vec3 fogColor = mix(horizonColor, zenithColor, skyHeight);
    out_Color.rgb = mix(out_Color.rgb, fogColor, fogAmount);
    out_Color.a = v_color.a;
}
