#version 150 core
in vec3 position;
out vec3 tex_cord;

uniform mat4 projection;
uniform mat4 view;

void main()
{ 
    tex_cord = position;
    gl_Position = projection * view * vec4(position, 1.0);
}