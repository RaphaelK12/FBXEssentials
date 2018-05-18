#version 330 core

layout(location = 0) in vec4 vertex;

//uniform mat4 model;
//uniform mat4 view;
//uniform mat4 projection;
uniform mat4 MVP;

void main()
{
	//gl_Position = projection*view*model*vec4(vertex,1.0f);
	gl_Position = MVP*vec4(vertex);
}