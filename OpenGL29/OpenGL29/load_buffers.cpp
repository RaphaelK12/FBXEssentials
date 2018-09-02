#include "Common.h"

std::vector<glm::vec3> vectorvertices, vectorvertices1;
std::vector<glm::vec2> vectoruvs, vectoruvs1;
std::vector<glm::vec3> vectornormals, vectornormals1;
int verticevectorsize, verticevectorsize1;
std::vector<unsigned short> indices, indices1;
std::vector<glm::vec3> indexed_vertices, indexed_vertices1;
std::vector<glm::vec2> indexed_uvs, indexed_uvs1;
std::vector<glm::vec3> indexed_normals, indexed_normals1;

extern GLuint MatrixID;

GLuint programID;
GLuint TextureID;
GLuint Texture;
GLuint VertexArrayID;
GLuint vertexbuffer;
GLuint uvbuffer;
GLuint normalbuffer;
GLuint elementbuffer;

void initVBO(char*modelfile, char*texturefile)
{
	loadShaders(programID, "shaders/vert_shader.glsl", "shaders/frag_shader.glsl");

	int imgwidth, imgheight;
	bool hasAlpha;
	//Load the texture
	Texture = loadPngImage(texturefile, imgwidth, imgheight, hasAlpha, &textureImage);
		//loadBMP_custom("test.bmp");
		//LoadBMP("test.bmp");
		//loadDDS("uvmap.DDS");
	
	//Get a handle for our "myTextureSampler" uniform
	TextureID = glGetUniformLocation(programID, "myTexture");
	MatrixID = glGetUniformLocation(programID, "MVP");

	//Get a handle for our "LightPosition" uniform
	//GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");
	
	char getFileExtension[64];
	for(int i = strlen(modelfile)-1; i >= 0; i--)
	{
		printf("strNum: %d\n", i);
		printf("strChar: %c\n", modelfile[i]);
		getFileExtension[i] = modelfile[i];
		if(strncmp(&getFileExtension[i],".obj", 4) == 0)//if(getFileExtension == ".obj")
		{
			printf(".obj file detected!\n");
			loadOBJ(modelfile, vectorvertices, vectoruvs, vectornormals);
			indexVBO(vectorvertices, vectoruvs, vectornormals, indices, indexed_vertices, indexed_uvs, indexed_normals);
			break;
		}
		if(strncmp(&getFileExtension[i],".fbx", 4) == 0)//if(getFileExtension == ".obj")
		{
			printf(".fbx file detected!\n");
			loadmyFBX(modelfile, vectorvertices, vectoruvs, vectornormals);
			indexVBO(vectorvertices, vectoruvs, vectornormals, indices, indexed_vertices, indexed_uvs, indexed_normals);
			break;
		}
	}
	
	/*
	for(int i = 0; i < 36; i++)
	{
		printf("%d polyV X: %f Y: %f Z: %f\n", i, vectorvertices1[i].x, vectorvertices1[i].y, vectorvertices1[i].z);
		printf("%d polyUV X: %f Y: %f\n", i, vectoruvs1[i].x, vectoruvs1[i].y);
		printf("%d polyN X: %f Y: %f Z: %f\n", i, vectornormals1[i].x, vectornormals1[i].y, vectornormals1[i].z);
	}
	*/
	
	//VAO
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);
	//VERTEXBUFFER
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, vectorvertices1.size() * sizeof(glm::vec3), &vectorvertices1[0], GL_STATIC_DRAW);
	//UVBUFFER
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_uvs.size() * sizeof(glm::vec2), &indexed_uvs[0], GL_STATIC_DRAW);
	//glBufferData(GL_ARRAY_BUFFER, vectoruvs1.size() * sizeof(glm::vec2), &vectoruvs1[0], GL_STATIC_DRAW);
	//NORMALBUFFER
	glGenBuffers(1, &normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);
	//INDICEBUFFERS
	glGenBuffers(1, &elementbuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0], GL_STATIC_DRAW);
}

void loadVBO()
{
	glUseProgram(programID);

	glActiveTexture(GL_TEXTURE0);//Bind our texture in Texture Unit 0
	glBindTexture(GL_TEXTURE_2D, Texture);
	glUniform1i(TextureID, 0);//Set "myTextureSampler" sampler to use Texture Unit 0

	//1st attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//Index buffer
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, (void*)0);
	//glDrawArrays(GL_TRIANGLES, 0, 36);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void deleteVBO()
{
	glDeleteBuffers(1, &uvbuffer);
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &normalbuffer);
	glDeleteBuffers(1, &elementbuffer);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);
	//glDeleteProgram(programID);
}