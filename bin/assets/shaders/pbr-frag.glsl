#version 450 core

#define COLOR                entity[v_in.instance_id].color
#define USE_ALBEDO_MAP       entity[v_in.instance_id].use_albedo_map
#define USE_PBR_MAPS         entity[v_in.instance_id].use_pbr_maps
#define REFLECTANCE          entity[v_in.instance_id].reflectance
#define ROUGHNESS            entity[v_in.instance_id].roughness
#define METALLIC             entity[v_in.instance_id].metallic
#define NORMAL_MAP_INTENSITY entity[v_in.instance_id].use_normal_map

#define LIGHT_DIRECTION      light_dir
#define LIGHT_INTENSITY      light_intensity
#define CAMERA_POSITION      camera_pos

#define ALBEDO_MAP           u_textures[entity[v_in.instance_id].albedo_map_index]
#define METALLIC_MAP         u_textures[entity[v_in.instance_id].metallic_map_index]
#define ROUGHNESS_MAP        u_textures[entity[v_in.instance_id].roughness_map_index]
#define NORMAL_MAP           u_textures[entity[v_in.instance_id].normal_map_index]

#define LUT_MAP              u_textures[lut_index]
#define IRRADIANCE_MAP       u_cubemaps[irr_index]
#define PREFILTER_MAP        u_cubemaps[pref_index]

const float PI = 3.14159265359;
const vec3  kFdielectric = vec3(0.04);
const float kMaxPrefilterLod = 6.0;

in Vertex
{
    mat3 tbn;
    vec3 position;
    vec2 uv;
    float instance_idf;
    flat int instance_id;
} v_in;

out vec4 FragColor;

struct EntityData
{
    mat4 model;
    vec3 color;
    float use_albedo_map;
    vec2 tiling;
    float use_pbr_maps;
    float reflectance;
    float roughness;
    float metallic;
    float use_normal_map;
    float _pad1;
    int albedo_map_index;
    float albedo_map_layer;
    int normal_map_index;
    float normal_map_layer;
    int roughness_map_index;
    float roughness_map_layer;
    int metallic_map_index;
    float metallic_map_layer;
};

layout(std140, binding=0) uniform u_data 
{
    mat4 vp;
    vec3 camera_pos;
    float _pad2;
    vec3 light_dir;
    float light_intensity;
    int lut_index;
    float lut_layer;
    int irr_index;
    float irr_layer;
    int pref_index;
    float pref_layer;
    float _pad3;
    float _pad4;
    EntityData entity[256];
};

layout(binding=8) uniform sampler2DArray u_textures[16];
layout(binding=0) uniform samplerCubeArray u_cubemaps[8];

// Normal distribution function (from filament documentation)
float Distribution_GGX(float noh, float roughness) {
    float a = noh * roughness;
    float k = roughness / (1.0 - noh * noh + a * a);
    return k * k * (1.0 / PI);
}

// Correlated Smith approximation (from filament documentation)
float Geometric_SmithGGX(float nol, float nov, float a) { // a is roughness
    float v = nol * (nov * (1.0 - a) + a);
    float l = nov * (nol * (1.0 - a) + a);
    return 0.5 / (v + l);
}

// Schlick's approximation
vec3 Fschlick(vec3 f0, float f90, float voh) {
    return f0 + (f90 - f0) * pow(1.0 - voh, 5.0);
}

vec3 Fresnel(vec3 f0, float voh)
{
    float f90 = clamp(dot(f0, vec3(50.0 * 0.33)), 0.0, 1.0);
    return Fschlick(f0, f90, voh);
}

float Lambert() {
    return 1.0 / PI;
}

float SpecularAA(vec3 n, float a)
{
    const float SIGMA2 = 0.25; // squared std dev of pixel filter kernel (in pixels)
    const float KAPPA  = 0.18; // clamping threshold

    vec3 dndu = dFdx(n);
    vec3 dndv = dFdy(n);
    float variance = SIGMA2 * (dot(dndu, dndu) + dot(dndv, dndv));
    float kernelRoughness2 = min(2.0 * variance, KAPPA);
    return clamp(a + kernelRoughness2, 0.045, 1.0);
}

void main()
{
    vec3 albedo = texture(ALBEDO_MAP, vec3(v_in.uv, entity[v_in.instance_id].albedo_map_layer)).rgb;
    albedo = mix(COLOR, albedo, USE_ALBEDO_MAP);
    float metalness = texture(METALLIC_MAP, vec3(v_in.uv, entity[v_in.instance_id].metallic_map_layer)).r;
    metalness = mix(METALLIC, metalness, USE_PBR_MAPS);
    float perceptual_roughness = texture(ROUGHNESS_MAP, vec3(v_in.uv, entity[v_in.instance_id].roughness_map_layer)).r;
    perceptual_roughness = mix(ROUGHNESS, perceptual_roughness, USE_PBR_MAPS);

    vec3 normal = normalize(2.0 * texture(NORMAL_MAP, vec3(v_in.uv, entity[v_in.instance_id].normal_map_layer)).rgb - 1.0);
    normal = normalize(v_in.tbn * normal);
    normal = mix(normalize(v_in.tbn[2]), clamp(normal, -1.0, 1.0), NORMAL_MAP_INTENSITY);

    vec3 f0 = 0.16 * REFLECTANCE * REFLECTANCE * (1.0 - metalness) + albedo * metalness;
    float f90 = clamp(dot(f0, vec3(50.0 * 0.33)), 0.0, 1.0);

    vec3 view = normalize(CAMERA_POSITION - v_in.position);
    vec3 light = -LIGHT_DIRECTION;
    // Half vec between light and view 
    vec3 hlv = normalize(light + view);

    float nov = abs(dot(normal, view));
    float nol = clamp(dot(normal, light), 0.0, 1.0);
    float noh = clamp(dot(normal, hlv), 0.0, 1.0);
    float voh = clamp(dot(view, hlv), 0.0, 1.0);

    float loh = clamp(dot(light, hlv), 0.0, 1.0);

    float roughness = perceptual_roughness * perceptual_roughness;
    vec3  f = Fschlick(f0, f90, loh);
    float d = Distribution_GGX(noh, roughness);
    float g = Geometric_SmithGGX(nol, nov, roughness);

    // Diffuse
    vec3 diffuse_color = vec3((1.0 - metalness) * albedo);
    vec3 diffuse_brdf = diffuse_color * Lambert();

    // Specular
    vec3 specular_brdf = ((d * g) * f);
    vec3 direct_lighting = (diffuse_brdf + specular_brdf) * LIGHT_INTENSITY * nol;

    // IBL
    vec3 r = reflect(-view, normal);
    float lod = perceptual_roughness * kMaxPrefilterLod;
    vec3 specular_irradiance = textureLod(PREFILTER_MAP, vec4(r, pref_layer), lod).rgb;
    vec2 dfg_lut = texture(LUT_MAP, vec3(nov, perceptual_roughness, lut_layer)).rg;
    vec3 ibl_dfg = mix(dfg_lut.xxx, dfg_lut.yyy, f0);
    vec3 specular_ibl = ibl_dfg * specular_irradiance;

    vec3 irradiance = texture(IRRADIANCE_MAP, vec4(r, irr_layer)).rgb;
    vec3 diffuse_ibl = irradiance * diffuse_color;

    vec3 ambient_lighting = diffuse_ibl + specular_ibl;

    FragColor = vec4(direct_lighting + ambient_lighting, 1.0);
}
