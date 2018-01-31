#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "libfbxsdk-mt.lib")
#pragma comment(lib, "libglew32.lib")
#pragma comment(lib, "freeglut_static.lib")
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <glew.h>
#include <freeglut.h>
#include <png.h>

#include "load_textures.h"

GLubyte*textureImage;
int image_num_texture = -1;

/**********************************************************
 * FUNCTION LoadBitmap(char *p_filename)
 * This function loads a bitmap file and return the OpenGL reference ID to use that texture
 * Input parameters: p_filename = Filename of the bitmap to load
 * Return value: (char) >-1 if the image was loaded correctly, -1 otherwise
 *********************************************************/

int loadPngImage(char*name, int&outWidth, int&outHeight, bool&outHasAlpha, GLubyte**outData)
{
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	int color_type, interlace_type;
	image_num_texture++;
	FILE*fp;
	if((fp = fopen(name, "rb")) == NULL) return false;
	/*Create and initialize the png_struct with the desired error handler functions.If you want to use the default
	stderr and longjump method, you can supply NULL for the last three parameters. We also supply the compiler header
	file version, so that we know if the application was compiled with a compatible version of the library REQUIRED*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(png_ptr == NULL)
	{
		fclose(fp);
		return false;
	}
    /*Allocate initialize the memory for image information REQUIRED*/
	info_ptr = png_create_info_struct(png_ptr);
	if(info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, NULL, NULL);
		return false;
	}
	/*Set error handling if you are using the setjmp/longjmp method (this is the normal method of doing things with
	libpng).REQUIRED unless you set up your own error handlers in the png_create_read_struct()earlier*/
	if(setjmp(png_jmpbuf(png_ptr)))
	{
		/*Free all of the memory associated with the png_ptr and info_ptr*/
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		/*If we get here, we had a problem reading the file*/
		return false;
	}
	/*Set up the output control if you are using standard C streams*/
	png_init_io(png_ptr, fp);
	/*If we have already read some of the signature*/
	png_set_sig_bytes(png_ptr, sig_read);
	/*If you have enough memory to read in the entire image at once, and you need to specify only
	transforms that can be controlled with one of the PNG_TRANSFORM_* bits (this presently excludes
	dithering, filling, setting background, and doing gamma adjustment), then you can read the entire image
	(including pixels)into the info structure with this call
	PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING  forces 8 bit PNG_TRANSFORM_EXPAND
	forces to expand a palette into RGB*/
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, NULL);
	png_uint_32 width, height;
	int bit_depth;
	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
	
	outHasAlpha = (color_type == PNG_COLOR_TYPE_RGBA);
	//outHasAlpha = (color_type & PNG_COLOR_MASK_ALPHA);
	//outHasAlpha = (color_type & PNG_COLOR_TYPE_GRAY_ALPHA);
	outWidth = width;
	outHeight = height;
	
	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*outData = (unsigned char*) malloc(row_bytes * outHeight);
	
	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
	for(int i = 0; i < outHeight; i++)
	{
		//note that png is ordered top to bottom, but OpenGL expect it bottom to top so the order or swapped
		memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
	}

	//glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, image_num_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, outHasAlpha ? 4 : 3, outWidth, outHeight, 0,
		outHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureImage);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	printf("Image load: %dx%d Alpha: %d\n", outWidth, outHeight, outHasAlpha);

	/* Clean up after the read, and free any memory allocated */
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
	fclose(fp);
	return(image_num_texture);
}
int LoadBMP(char*p_filename) 
{
	int i, j = 0; //Index variables
	FILE*l_file; //File pointer
	unsigned char*l_texture; //The pointer to the memory zone in which we will load the texture
	// windows.h gives us these types to work with the Bitmap files
	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;
	RGBTRIPLE rgb;
	image_num_texture++; // The counter of the current texture is increased
	if(p_filename=='\0')return(-1);
	if((l_file = fopen(p_filename, "rb"))==NULL)return(-1);
	
	fread(&fileheader, sizeof(fileheader), 1, l_file); // Read the fileheader
	fseek(l_file, sizeof(fileheader), SEEK_SET); // Jump the fileheader
	fread(&infoheader, sizeof(infoheader), 1, l_file); // and read the infoheader
	//Now we need to allocate the memory for our image (width * height * color deep)
	l_texture = (byte*)malloc(infoheader.biWidth*infoheader.biHeight*4);
	//And fill it with zeros
	memset(l_texture, 0, infoheader.biWidth * infoheader.biHeight * 4);
	//At this point we can read every pixel of the image
	for(i=0; i < infoheader.biWidth*infoheader.biHeight; i++)
	{
		//We load an RGB value from the file
		fread(&rgb, sizeof(rgb), 1, l_file);
		//And store it
		l_texture[j+0] = rgb.rgbtRed; // Red component
		l_texture[j+1] = rgb.rgbtGreen; // Green component
		l_texture[j+2] = rgb.rgbtBlue; // Blue component
		l_texture[j+3] = 255; // Alpha value
		j += 4; // Go to the next position
	}
	fclose(l_file); //Closes the file stream
	glBindTexture(GL_TEXTURE_2D, image_num_texture); //Bind the ID texture specified by the 2nd parameter
	// The next commands sets the texture parameters
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT); //If the u,v coordinates overflow the range 0,1 the image is repeated
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //The magnification function("linear" produces better results)
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST); //The minifying function
	
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); //We don't combine the color with the original surface color, use only the texture map.
	//Finally we define the 2d texture
	glTexImage2D(GL_TEXTURE_2D, 0, 4, infoheader.biWidth, infoheader.biHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
	//And create 2d mipmaps for the minifying function
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, infoheader.biWidth, infoheader.biHeight, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
	free(l_texture); //Free the memory we used to load the texture
	return(image_num_texture); //Returns the current texture OpenGL ID
}