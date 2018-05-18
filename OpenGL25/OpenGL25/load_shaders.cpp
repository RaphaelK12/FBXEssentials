#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment (lib, "libglew32.lib")
#pragma comment(lib, "libglew32.lib")
#pragma comment(lib, "freeglut_static.lib")
#include <stdio.h>
#include <stdlib.h>
#include <glew.h>
#include <freeglut.h>
//#include <iostream>
//#include <fstream>
#include "load_shaders.h"

GLuint vert_hdlr, frag_hdlr;

//char*copy;
//char*copypointer[] = {copy};

void printInfoLog(GLuint obj)
{
	int log_size = 0;
	int bytes_written = 0;
	glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &log_size);
	if(!log_size)return;
	char*infoLog = new char[log_size];
	glGetProgramInfoLog(obj, log_size, &bytes_written, infoLog);
	printf("%s\n", infoLog);
	delete[]infoLog;
}
bool read_n_compile_shader(const char *filename, GLuint &hdlr, GLenum shaderType)
{
	FILE*file;
	fopen_s(&file, filename, "rb");
	if(!file)
	{
		perror("ERROR: Cannot open file\n");
		return "\0";
	}
	fseek(file, 0L, SEEK_END); //Tell file size
	unsigned long shaderFileSize;
	shaderFileSize = ftell(file);
	//printf("DEBUG shaderFileSize: %lu\n", shaderFileSize);
	rewind(file);
	char*buffer = (char*)malloc(sizeof(char)*(shaderFileSize+1UL));
	if(!buffer)
	{
		perror("ERROR: Failed to allocate memory\n");
		return "\0";
	}
	int c, i = 0;
	while((c = fgetc(file))!= EOF)buffer[i++] = c;
	buffer[shaderFileSize] = '\0'; //Put '\0' at the end of the buffer(required for OpenGL)
	//printf("Buffer:\n%s\n", buffer);
	//printf("strlen: %d\n", strlen(buffer));
	fclose(file);
	hdlr = glCreateShader(shaderType);
	glShaderSource(hdlr, 1, (const GLchar**)&buffer, NULL);
	glCompileShader(hdlr);
	printf("Loading info for: %s\n", filename);
	
	/*
	copy = (char*)malloc(sizeof(char)*(shaderFileSize+1UL));
	strcpy(copy, buffer);

	copypointer[2] = copy;
	printf("COPYTEST: %s\n", copypointer[2]);
	*/

	//printInfoLog(hdlr);
	delete[]buffer;
	return true;
}
void loadShaders(GLuint &prog_hdlr, const char*vsfile, const char*fsfile)
{
	/*
	//size_t shaderNameSize = strlen(shaderStr);
	const char*shaderStr = vsfile;
	int shaderNameSize = 0;
	while(shaderStr[shaderNameSize] != '\0')shaderNameSize++;

	char*copytest = (char*)malloc(sizeof(char)*(shaderNameSize+1UL));
	printf("SHADERNAMESIZE: %d\n", shaderNameSize);
	getchar();

	char*copypointertest[] = {copytest};

	strcpy(copytest, vsfile);
	copypointertest[0] = copytest;
	strcpy(copytest, fsfile);
	copypointertest[1] = copytest;

	printf("COPYTEST: %s - %s\n", copypointertest[0], copypointertest[1]);
	*/
	
	read_n_compile_shader(vsfile, vert_hdlr, GL_VERTEX_SHADER);
	read_n_compile_shader(fsfile, frag_hdlr, GL_FRAGMENT_SHADER);
	prog_hdlr = glCreateProgram();
	glAttachShader(prog_hdlr, frag_hdlr);
	glAttachShader(prog_hdlr, vert_hdlr);
	glLinkProgram(prog_hdlr);
	//printf("Loading linked program: vs: %d fs: %d prog_hdlr: %d\n", vert_hdlr, frag_hdlr, prog_hdlr);
	glUseProgram(prog_hdlr);
	//printInfoLog(prog_hdlr);

	glDetachShader(prog_hdlr, vert_hdlr);
	glDetachShader(prog_hdlr, frag_hdlr);
	//glDeleteShader(vert_hdlr);
	//glDeleteShader(frag_hdlr);
}

void setShaders(GLuint &set_prog_hdlr, GLuint set_vert_hdlr, GLuint set_frag_hdlr)
{
	//set_prog_hdlr = glCreateProgram();
	glAttachShader(set_prog_hdlr, set_frag_hdlr);
	glAttachShader(set_prog_hdlr, set_vert_hdlr);
	glLinkProgram(set_prog_hdlr);
	//printf("SETSHADER: vs: %d fs: %d prog_hdlr: %d\n", set_vert_hdlr, set_frag_hdlr, set_prog_hdlr);
	glUseProgram(set_prog_hdlr);
	//printInfoLog(set_prog_hdlr);
	glDetachShader(set_prog_hdlr, set_vert_hdlr);
	glDetachShader(set_prog_hdlr, set_frag_hdlr);
}



/*
	std::ifstream is(filename, std::ios::in|std::ios::binary|std::ios::ate);	
	if(!is.is_open())
	{
		printf("Unable to open file %s\n", filename);
		return false;
	}
	long size = (long)is.tellg();
	char*buffer = new char[size+1];
	is.seekg(0, std::ios::beg);
	is.read (buffer, size);
	is.close();
	buffer[size] = 0;
*/