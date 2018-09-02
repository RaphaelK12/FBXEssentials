#version 330 core

in vec2 UV;

out vec3 FragColor;

uniform sampler2D myTexture;

void main()
{
	FragColor = texture(myTexture, UV).rgb;
}