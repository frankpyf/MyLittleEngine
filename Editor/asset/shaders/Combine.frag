#version 450

layout(set = 0, binding = 0) uniform sampler2D sky_texture;

layout(location = 0) in vec4 world_position;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;

vec3 ACESFilm(vec3 x)
{
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

void main()
{
    vec4 sky = texture(sky_texture, tex_coord);

    vec3 toned_color = ACESFilm(sky.rgb);

    out_color = vec4(toned_color, 1);
}