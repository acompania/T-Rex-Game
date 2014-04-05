#ifndef __TEXTURE_LOADER_H__
#define __TEXTURE_LOADER_H__

/*data structure for the image used for  texture mapping */
typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

typedef struct RGB {
  GLubyte r;
  GLubyte g; 
  GLubyte b;
} RGB;


GLvoid LoadTexture(char* image_file, int texID, Image * image);
int ImageLoad(char *filename, Image *image);


#endif