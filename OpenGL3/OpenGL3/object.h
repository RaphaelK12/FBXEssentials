#ifndef _OBJECT_H
#define _OBJECT_H
#define MAX_VERTICES 800000 //Max number of vertices(for each object)
#define MAX_POLYGONS 800000 //Max number of polygons(for each object)
#define MAX_OBJECTS 100 //Max number of objects
char name[20]; //Name of the object
int id_texture;
int vertices_quantity; //Number of vertices
int polygons_quantity; //Number of polygons
// Our vertex type
struct vertex
{
	float x, y, z;
}vertices[MAX_VERTICES], normal[MAX_VERTICES], test[MAX_VERTICES];
//Array of vertices normals
// The polygon(triangle), 3 numbers that aim 3 vertices
struct
{
	unsigned short a, b, c;
}polygon[MAX_POLYGONS];
//Array of polygons (numbers that point to the vertices' list)
// The mapcoord type, 2 texture coordinates for each vertex
struct
{
	float u, v;
}mapcoord[MAX_VERTICES];
// Array of U,V coordinates for texture mapping



#endif