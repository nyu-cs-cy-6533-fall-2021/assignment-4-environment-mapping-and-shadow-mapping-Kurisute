#version 150 core
in vec3 position;
uniform mat4 light_perspective;
uniform mat4 light_view;
uniform mat4 model;
void main()
{
    gl_Position = light_perspective * light_view * model * vec4(position, 1.0);
}