#define _CRT_SECURE_NO_WARNINGS
#define GLEW_STATIC
#define FREEGLUT_STATIC
#define _LIB
#define FREEGLUT_LIB_PRAGMAS 0
#pragma comment(lib, "libpng16.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glu32.lib")
#pragma comment(lib, "libglew32.lib")
#pragma comment(lib, "freeglut_static.lib")
#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <glew.h>
#include <freeglut.h>
#include <png.h>

#include "Common.h"

GLubyte*textureImage;
GLuint image_num_texture = -1;

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


	glGenTextures(1, &image_num_texture);
	glBindTexture(GL_TEXTURE_2D, image_num_texture);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	//glTexImage2D(GL_TEXTURE_2D, 0, outHasAlpha ? 4 : 3, outWidth, outHeight, 0,
		//outHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureImage);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0,
		outHasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, textureImage);


	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	
	/*
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	*/
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);
	

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
	glTexImage2D(GL_TEXTURE_2D, 0, 4, infoheader.biWidth, infoheader.biHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, l_texture);
	//And create 2d mipmaps for the minifying function
	gluBuild2DMipmaps(GL_TEXTURE_2D, 4, infoheader.biWidth, infoheader.biHeight, GL_RGBA, GL_UNSIGNED_BYTE, l_texture);
	free(l_texture); //Free the memory we used to load the texture
	return(image_num_texture); //Returns the current texture OpenGL ID
}



GLuint loadBMP_custom(const char * imagepath)
{
	printf("Reading image %s\n", imagepath);
	// Data read from the header of the BMP file
	unsigned char header[54];
	unsigned int dataPos;
	unsigned int imageSize;
	unsigned int width, height;
	// Actual RGB data
	unsigned char * data;

	image_num_texture++;

	// Open the file
	FILE * file = fopen(imagepath,"rb");
	if (!file){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath);
		getchar();
		return 0;
	}

	// Read the header, i.e. the 54 first bytes

	// If less than 54 bytes are read, problem
	if ( fread(header, 1, 54, file)!=54 ){ 
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// A BMP files always begins with "BM"
	if ( header[0]!='B' || header[1]!='M' ){
		printf("Not a correct BMP file\n");
		fclose(file);
		return 0;
	}
	// Make sure this is a 24bpp file
	if ( *(int*)&(header[0x1E])!=0  )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}
	if ( *(int*)&(header[0x1C])!=24 )         {printf("Not a correct BMP file\n");    fclose(file); return 0;}

	// Read the information about the image
	dataPos    = *(int*)&(header[0x0A]);
	imageSize  = *(int*)&(header[0x22]);
	width      = *(int*)&(header[0x12]);
	height     = *(int*)&(header[0x16]);

	// Some BMP files are misformatted, guess missing information
	if (imageSize==0)    imageSize=width*height*3; // 3 : one byte for each Red, Green and Blue component
	if (dataPos==0)      dataPos=54; // The BMP header is done that way

	// Create a buffer
	data = new unsigned char [imageSize];

	// Read the actual data from the file into the buffer
	fread(data,1,imageSize,file);

	// Everything is in memory now, the file can be closed.
	fclose (file);

	// Create one OpenGL texture
	//GLuint textureID;
	glGenTextures(1, &image_num_texture); //textureID;
	
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, image_num_texture); //textureID;

	// Give the image to OpenGL
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

	// OpenGL has now copied the data. Free our own version
	delete [] data;

	// Poor filtering, or ...
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 

	// ... nice trilinear filtering ...
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	// ... which requires mipmaps. Generate them automatically.
	glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return image_num_texture;
}


#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath)
{
	unsigned char header[124];
	FILE *fp;
	image_num_texture++;
	/* try to open the file */ 
	fp = fopen(imagepath, "rb"); 
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); 
		return 0;
	}
	/* verify the type of file */ 
	char filecode[4]; 
	fread(filecode, 1, 4, fp); 
	if (strncmp(filecode, "DDS ", 4) != 0) { 
		fclose(fp); 
		return 0; 
	}
	/* get the surface desc */ 
	fread(&header, 124, 1, fp); 

	unsigned int height      = *(unsigned int*)&(header[8 ]);
	unsigned int width	     = *(unsigned int*)&(header[12]);
	unsigned int linearSize	 = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC      = *(unsigned int*)&(header[80]);
 
	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */ 
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize; 
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char)); 
	fread(buffer, 1, bufsize, fp); 
	/* close the file pointer */ 
	fclose(fp);

	unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4; 
	unsigned int format;
	switch(fourCC) 
	{ 
	case FOURCC_DXT1: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
		break; 
	case FOURCC_DXT3: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
		break; 
	case FOURCC_DXT5: 
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
		break; 
	default: 
		free(buffer); 
		return 0; 
	}

	// Create one OpenGL texture
	//GLuint textureID;
	glGenTextures(1, &image_num_texture);//&textureID);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, image_num_texture);//textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	
	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; 
	unsigned int offset = 0;

	/* load the mipmaps */ 
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) 
	{ 
		unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize; 
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,  
			0, size, buffer + offset); 
	 
		offset += size; 
		width  /= 2; 
		height /= 2; 

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if(width < 1) width = 1;
		if(height < 1) height = 1;

	} 
	free(buffer); 
	return image_num_texture;//textureID;
}