#version 450 core

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

layout(std140, binding=30) uniform BlockData
{
    EntityData entity[1024];
};

layout(std140, binding=10) uniform SharedData
{
    mat4 vp;
    vec3 camera_pos;
    float _pad2;
    vec3 light_dir;
    float light_intensity;
};

layout(location=0) in vec3 a_position;
layout(location=1) in vec3 a_normal;
layout(location=2) in vec3 a_tangent;
layout(location=3) in vec3 a_bitangent;
layout(location=4) in vec2 a_uv;

out Vertex
{
    mat3 tbn;
    vec3 position;
    vec2 uv;
    float instance_idf;
    flat int instance_id;
} v_out;

void main()
{
    v_out.position = vec3(entity[gl_InstanceID].model * vec4(a_position, 1.0));
    v_out.uv = vec2(a_uv.x * entity[gl_InstanceID].tiling.x, a_uv.y * entity[gl_InstanceID].tiling.y);

    v_out.instance_id = gl_InstanceID;
    v_out.instance_idf = float(gl_InstanceID);

    // Pass tangent space basis vectors (for normal mapping).
    v_out.tbn = mat3(entity[gl_InstanceID].model) * mat3(normalize(a_tangent), normalize(a_bitangent), normalize(a_normal));

    gl_Position = vp * entity[gl_InstanceID].model * vec4(a_position, 1.0);
}
