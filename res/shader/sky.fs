#version 450 core

in vec2 v_clipPosition;

out vec4 out_Color;

uniform mat4 u_inverseView;
uniform mat4 u_inverseProjection;
uniform vec3 u_sunDirection;

void main() {
    vec4 viewPosition =
        u_inverseProjection * vec4(v_clipPosition, 1.0, 1.0);
    vec3 viewDirection = normalize(viewPosition.xyz / viewPosition.w);
    vec3 worldDirection = normalize(
        (u_inverseView * vec4(viewDirection, 0.0)).xyz);

    float horizon = smoothstep(-0.18, 0.55, worldDirection.y);
    vec3 horizonColor = vec3(0.52, 0.66, 0.86);
    vec3 zenithColor = vec3(0.15, 0.28, 0.58);
    vec3 skyColor = mix(horizonColor, zenithColor, horizon);

    float alignment = dot(worldDirection, normalize(u_sunDirection));
    float sunGlow = smoothstep(0.985, 1.0, alignment);
    float sunDisc = smoothstep(0.99925, 0.99965, alignment);
    skyColor += vec3(1.0, 0.62, 0.20) * sunGlow * 0.28;
    skyColor = mix(skyColor, vec3(1.0, 0.91, 0.62), sunDisc);

    out_Color = vec4(skyColor, 1.0);
}
