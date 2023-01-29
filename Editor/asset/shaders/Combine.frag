#version 450

layout(set = 0, binding = 1) uniform sampler2D triangle_texture;

layout(input_attachment_index = 0, set = 0, binding = 2) uniform subpassInput sky_texture;

layout(location = 0) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

void main()
{
    vec4 sky = subpassLoad(sky_texture).rgba;
    vec4 triangle = texture(triangle_texture, tex_coord).rgba;

    out_color = mix(sky, triangle, 0.2);
}