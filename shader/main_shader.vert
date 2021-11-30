#version 150 core
in vec3 position;
in vec3 color;
in vec3 normal;
out vec3 f_position;
out vec4 f_position_lightspace;
out vec3 f_color;
out vec3 f_normal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 perspective;
uniform mat4 light_perspective;
uniform mat4 light_view;
void main()
{
    f_position = vec3(model * vec4(position, 1.0));
    gl_Position = perspective * view * model * vec4(position, 1.0);
    f_position_lightspace = light_perspective * light_view * model * vec4(position, 1.0);
    f_color = color;
    f_normal = mat3(transpose(inverse(model))) * normal;
}