layout(std140, binding=0) uniform u_data
{
    int img_index;
    float img_layer;
};

in vec2 uv;

out vec4 FragColor;

void main()
{
    vec3 color = texture(u_textures[img_index], vec3(uv, img_layer)).rgb;

    // Tone mapping
    color = color / (color + vec3(1.0));

    // gamma correction
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color,  1.0);
}
