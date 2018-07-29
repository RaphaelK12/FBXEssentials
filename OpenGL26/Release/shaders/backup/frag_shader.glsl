#version 330 core

out vec3 color;

void main()
{
	float lin = 1.0/gl_FragCoord.w;
	float depth = (lin - 0.5)/(16.0 - 0.5);
	color = vec3(depth, depth, 1.0f);
}