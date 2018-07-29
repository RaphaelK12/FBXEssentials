#define _CRT_SECURE_NO_WARNINGS
#include <vector>
#include <stdio.h>
#include <string>
#include <cstring>
#include <conio.h>

#include <glm/glm.hpp>
#include "fbxloader.h"

extern int*fbxindices;
extern int fbxuvIndex[10000];
extern int fbxnormalIndex[10000];

extern int numIndices;
extern int TotalNumVerts;

extern float myVertices[10000];
extern float myUVmap[100000];
extern float myNormals[100000];

bool loadmyFBX(
	std::vector<glm::vec3> & out_vertices, 
	std::vector<glm::vec2> & out_uvs,
	std::vector<glm::vec3> & out_normals
	)
{
	std::vector<glm::vec3> temp_vertices; 
	std::vector<glm::vec2> temp_uvs;
	std::vector<glm::vec3> temp_normals;
	for(int i = 0; i < TotalNumVerts; i++)
	{
		//printf("I: %d / %d\n", i, TotalNumVerts);
		//printf("myVERT X: %f Y: %f Z: %f W: %f\n",
			//myVertices[i*4+0], myVertices[i*4+1], myVertices[i*4+2], myVertices[i*4+3]);
		glm::vec3 vertex;
		vertex.x = myVertices[i*3+0];
		vertex.y = myVertices[i*3+1];
		vertex.z = myVertices[i*3+2];
		//vertex.w = myVertices[i*4+3];
		printf("%d VERTX X: %f Y: %f Z: %f\n",
			i, vertex.x, vertex.y, vertex.z);
		temp_vertices.push_back(vertex);
	}
	for(int ii = 0; ii < numIndices; ii++)
	{
		//printf("NUMIND: %d / %d\n", ii, numIndices);
		glm::vec2 uv;
		uv.x = myUVmap[ii*2+0];
		uv.y = myUVmap[ii*2+1];
		printf("fbxuvIND: %d VECUV U: %f V: %f\n", fbxuvIndex[ii], uv.x, uv.y);
		temp_uvs.push_back(uv);

		glm::vec3 normal;
		normal.x = myNormals[ii*3+0];
		normal.y = myNormals[ii*3+1];
		normal.z = myNormals[ii*3+2];
		//printf("VECNORM X: %f Y: %f Z: %f\n",
			//normal.x, normal.y, normal.z);
		temp_normals.push_back(normal);
	}

	// For each vertex of each triangle
	for(unsigned int iii = 0; iii < numIndices; iii++)
	{
		// Get the indices of its attributes
		unsigned int vertexIndex = fbxindices[iii];
		unsigned int uvIndex = fbxuvIndex[iii];
		unsigned int normalIndex = fbxnormalIndex[iii];
		
		printf("vindX: %d uvI: %d nInd: %d\n", fbxindices[iii], fbxuvIndex[iii], fbxnormalIndex[iii]);


		// Get the attributes thanks to the index
		glm::vec3 vertex = temp_vertices[vertexIndex];
		glm::vec2 uv = temp_uvs[uvIndex];
		glm::vec3 normal = temp_normals[normalIndex];

		// Put the attributes in buffers
		out_vertices.push_back(vertex);
		out_uvs     .push_back(uv);
		out_normals .push_back(normal);
	}
	_getch();
	
	return true;
}