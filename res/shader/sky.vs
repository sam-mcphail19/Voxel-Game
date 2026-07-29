#version 450 core

out vec2 v_clipPosition;

void main() {
    const vec2 positions[3] = vec2[3](
        vec2(-1.0, -1.0),
        vec2( 3.0, -1.0),
        vec2(-1.0,  3.0)
    );

    v_clipPosition = positions[gl_VertexID];
    gl_Position = vec4(v_clipPosition, 1.0, 1.0);
}
