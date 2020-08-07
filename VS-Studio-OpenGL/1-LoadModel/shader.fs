#version 330 core
out vec4 FragColor;

uniform sampler2D diffTex;

in vec3 Normal;
in vec2 TexCoord;

void main()
{
    FragColor = texture(diffTex, TexCoord);
    //FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}