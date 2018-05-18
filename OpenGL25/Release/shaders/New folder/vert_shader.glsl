#version 330 core

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec2 vertexUV;
layout(location = 2) in vec3 vertexNormal;

out vec2 UV;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;

uniform mat4 MVP;

void main()
{
	//gl_Position = projection*view*model*vec4(vertex,1.0f);
	gl_Position = MVP*vec4(vertex,1.0f);
	UV = vertexUV;
}