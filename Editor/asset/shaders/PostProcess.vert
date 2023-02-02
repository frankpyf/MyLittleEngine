#version 450

layout(location = 0) out vec4 world_position;
layout(location = 1) out vec2 tex_coord;

void main() {
    // Full screen Triangle
    vec2 uv = vec2((gl_VertexIndex << 1) & 2, gl_VertexIndex & 2);
    gl_Position = vec4(uv * 2.0 - 1.0, 1.0, 1.0);

    world_position = gl_Position;
    tex_coord = uv;
}