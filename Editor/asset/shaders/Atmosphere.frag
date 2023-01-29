#version 450

const float PI = 3.14159265359;

#define SAMPLE_STEPS 32

layout(set = 0, binding = 0) uniform CameraComponent {
    mat4 inverse_view;
    mat4 inverse_proj;
    vec3 position;
} camera;

layout(set = 0, binding = 1) uniform AtmosphereParameter
{
    vec3 light_dir;
    float SunLightIntensity;
    vec3 SunLightColor;
    float SeaLevel;
    vec3 PlanetCenter;
    float PlanetRadius;
    float AtmosphereHeight;
    float SunDiskAngle;
    float RayleighScatteringScale;
    float RayleighScatteringScalarHeight;
    float MieScatteringScale;
    float MieAnisotropy;
    float MieScatteringScalarHeight;
    float OzoneAbsorptionScale;
    float OzoneLevelCenterHeight;
    float OzoneLevelWidth;
} param;

layout(location = 0) in vec4 world_position;
layout(location = 1) in vec2 tex_coord;

layout(location = 0) out vec4 out_color;


// Helper functions
vec3 ScatterCoefficient(vec3 sigma, float scalar_height, float h)
{
    return sigma * exp(-(h/scalar_height));
}

float RayleighPhase(float cos_theta)
{
    return (3.0 / (16.0 * PI)) * (1.0 + cos_theta * cos_theta);
}

float MiePhase(float g, float cos_theta)
{
    float a = 3.0 / (8.0 * PI);
    float gg = g * g;
    float b = (1.0 - gg) / (2.0 + gg);
    float c = 1.0 + cos_theta*cos_theta;
    float d = pow(1.0 + gg - 2*g*cos_theta, 1.5);

    return a * b * (c / d);
}

vec3 Scattering(float planet_radius, float rayleigh_scalar_height, float mie_scalar_height, float g, vec3 position, vec3 in_dir, vec3 out_dir)
{
    float cos_theta = dot(in_dir, out_dir);

    float h = length(position) - planet_radius;

    vec3 rayleigh = ScatterCoefficient(vec3(5.802, 13.558, 33.1) * 1e-6, rayleigh_scalar_height, h) * RayleighPhase(cos_theta);
    vec3 mie = ScatterCoefficient(vec3(3.996, 3.996, 3.996) * 1e-6 , mie_scalar_height, h) * MiePhase(g, cos_theta);

    return rayleigh + mie;
}

// this function is identical to the coefficient function above
// Rewrite it to disciminate Scattering from absorption
vec3 MieAbsorption(vec3 sigma, float scalar_height, float h)
{
    return sigma * exp(-(h / scalar_height));
} 

vec3 OzoneAbsorption(vec3 sigma, float ozone_center_height, float ozone_width, float h)
{
    return sigma * max(0, 1.0 - (abs(h - ozone_center_height) / ozone_width));
}

vec3 Transmittance(float planet_radius,
                   float rayleigh_scalar_height, 
                   float mie_scalar_height,
                   float ozone_center_height, 
                   float ozone_width, 
                   vec3 p1,
                   vec3 p2)
{
    vec3 dir = normalize(p2 - p1);
    float dis = length(p2 - p1);
    float delta_step = dis / float(SAMPLE_STEPS);
    vec3 sum = vec3(0.0);
    vec3 p = p1 + (dir * delta_step) * 0.5;
    
    for(int i = 0; i< SAMPLE_STEPS; i++)
    {
        float h = length(p) - planet_radius;

        vec3 scattering = ScatterCoefficient(vec3(5.802, 13.558, 33.1) * 1e-6, rayleigh_scalar_height, h) + ScatterCoefficient(vec3(3.996, 3.996, 3.996) * 1e-6 , mie_scalar_height, h);
        vec3 absorption = OzoneAbsorption(vec3(0.0650, 1.881, 0.085) * 1e-6, ozone_center_height, ozone_width, h) + MieAbsorption(vec3(4.4 * 1e-6), mie_scalar_height, h);
        vec3 extinction = scattering + absorption;

        sum += extinction * delta_step;
        p += dir * delta_step;
    }

    return exp(-sum);
}

// Returns dstToSphere and dstThroughSphere
vec2 RayIntersectSphere(vec3 center, float radius, vec3 start, vec3 dir)
{
    vec3 origin = start - center;

    float a = dot(dir, dir);
    float b = 2.0 * dot(origin, dir);
    float c = dot(origin, origin) - radius * radius;

    float d = b * b - 4.0 * a * c;
    if(d < 0)
        return vec2(0, 0);

    float s = sqrt(d);
    float dst1 = max(0.0, (-b - s)/(2.0*a));
    float dst2 = max(0.0, (-b + s)/(2.0*a));

    return vec2(dst1, dst2 - dst1);
}

// Main function
void main()
{
    // Rebuild World Position
    vec3 start_position = camera.position;
    start_position.y += param.PlanetRadius + 1.7;
    start_position.y -= param.SeaLevel;

    // Calculate current view direction
    vec4 pixel_world_position = camera.inverse_view * camera.inverse_proj *  world_position;
    vec3 view_dir = normalize(pixel_world_position.xyz / pixel_world_position.w - camera.position);

    vec3 light_dir = normalize(param.light_dir);

    vec3 color = vec3(0, 0, 0);

    // Intersect with atmosphere and planet
    vec2 intersection_atmosphere_info = RayIntersectSphere(param.PlanetCenter, param.PlanetRadius + param.AtmosphereHeight, start_position, view_dir);
    vec2 intersection_planet_info = RayIntersectSphere(param.PlanetCenter, param.PlanetRadius, start_position, view_dir);

    if(intersection_atmosphere_info.y == 0) 
    {
        out_color = vec4(color, 1.0);
        return;
    }

    // sample distance
    float dis = intersection_atmosphere_info.y;

    if(intersection_planet_info.x > intersection_atmosphere_info.x)
        dis = intersection_planet_info.x;

    float delta_step = dis / float(SAMPLE_STEPS);

    vec3 p1 = start_position + view_dir * intersection_atmosphere_info.x + view_dir * delta_step * 0.5;

    vec3 sun_luminance = param.SunLightColor * param.SunLightIntensity;

    for(int i = 0; i < SAMPLE_STEPS; i++)
    {
        // from P1 to Sun
        vec2 atmosphere_to_sun_info = RayIntersectSphere(param.PlanetCenter, param.PlanetRadius + param.AtmosphereHeight, p1, light_dir);
        vec3 p2 = p1 + light_dir * atmosphere_to_sun_info.y;

        vec3 t1 = Transmittance(param.PlanetRadius, param.RayleighScatteringScalarHeight, param.MieScatteringScalarHeight, param.OzoneLevelCenterHeight, param.OzoneLevelWidth, p2, p1);
        vec3 s = Scattering(param.PlanetRadius, param.RayleighScatteringScalarHeight, param.MieScatteringScalarHeight, param.MieAnisotropy, p1, light_dir, view_dir);
        vec3 t2 = Transmittance(param.PlanetRadius, param.RayleighScatteringScalarHeight, param.MieScatteringScalarHeight, param.OzoneLevelCenterHeight, param.OzoneLevelWidth, p1, start_position);

        vec3 in_scattering = t1 * s * t2 * delta_step * sun_luminance;
        color += in_scattering;
        p1 += view_dir * delta_step; 
    }

    // if(dot(view_dir, light_dir) < param.SunDiskAngle) color += sun_luminance;

    out_color = vec4(color, 1.0);
}