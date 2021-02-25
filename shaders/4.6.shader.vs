#version 460 core
layout (location = 0) in vec3 aPos;
uniform vec3 ourColor;
out vec4 color;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
    color = ourColor;
}