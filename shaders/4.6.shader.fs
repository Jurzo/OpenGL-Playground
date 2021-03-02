#version 460 core
out vec4 fragColor;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform sampler2D texture2;

void main()
{
    fragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord * vec2(-1, 1)), 0.4);
}