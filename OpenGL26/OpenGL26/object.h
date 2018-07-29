#ifndef _OBJECT_H
#define _OBJECT_H

#define MAX_VERTICES 800000 //Max number of vertices(for each object)
#define MAX_POLYGONS 800000 //Max number of polygons(for each object)
#define MAX_UV 800000 //The mapcoord type, 2 texture coordinates for each vertex

float vtx[MAX_VERTICES];

struct vertex
{
	float x, y, z, w;
}vertices[MAX_VERTICES], vNormal[MAX_VERTICES], test[MAX_VERTICES];
//Array of vertices & normals
//The polygon(triangle), 3 numbers that aim 3 vertices
struct polygon
{
	unsigned short a, b, c;
}polygons[MAX_POLYGONS];
//Array of polygons (numbers that point to the vertices' list)
struct map
{
	float u[123][3], v[123][3];
}mapcoords;

#endif