#version 450

layout(std140, binding=1) uniform BlockData
{
    mat4 model;
    int sky_index;
    float sky_layer;
};

uniform samplerCubeArray u_cubemaps[8];

out vec4 FragColor;
in vec3 position;

void main() 
{
    FragColor = texture(u_cubemaps[sky_index], vec4(position, sky_layer));
}
