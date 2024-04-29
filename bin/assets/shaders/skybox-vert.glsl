#version 450 core

layout(std140, binding=0) uniform u_data 
{
    mat4 model;
    int sky_index;
    float sky_layer;
};

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec2 a_uv;

out vec3 position;

void main()
{
    position = a_position;
    gl_Position = vec4(model * vec4(position, 1.0)).xyww;
}
