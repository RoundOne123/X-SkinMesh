#version 330 core
out vec4 FragColor;

in vec2 Texcoord;

uniform sampler2D diffTex;

void main()
{
    FragColor = texture(diffTex, Texcoord);
}