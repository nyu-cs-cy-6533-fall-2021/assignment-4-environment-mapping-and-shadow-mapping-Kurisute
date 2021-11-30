#version 150 core
in vec3 tex_cord;
out vec4 outColor;
uniform samplerCube skybox;

void main()
{
    outColor = texture(skybox, tex_cord);
}