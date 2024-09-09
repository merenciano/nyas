layout(std140, binding=0) uniform u_data 
{
    mat4 model;
    int sky_index;
    float sky_layer;
};

//layout(binding=0) uniform samplerCubeArray u_cubemaps[8];

out vec4 FragColor;
in vec3 uv;

void main() 
{
    FragColor = texture(u_cubemaps[sky_index], vec4(uv, sky_layer));
}
