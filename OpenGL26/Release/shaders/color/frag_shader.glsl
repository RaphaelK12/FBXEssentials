#version 330 core

out vec4 theColor;
in vec3 outColor;

void main()
{
	theColor = vec4(outColor, 1.0);
}