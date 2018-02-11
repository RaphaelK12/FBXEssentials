#ifndef _LOAD_TEXTURES_H
#define _LOAD_TEXTURES_H
#include <glew.h>
#include <freeglut.h>

extern GLubyte*textureImage;

extern int LoadBMP(char*p_filename);
extern int loadPngImage(char*name, int&outWidth, int&outHeight, bool&outHasAlpha, GLubyte**outData);
#endif