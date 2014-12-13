#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

typedef struct _Point2D{
  int x;
  int y;
} Point2D;

//a structure that holds the 
typedef struct _Component {
  int xEntry;
  int yEntry;
  int area;

} Component;

void openImage(bool** image, int height, int width);
void closeImage(bool** image, int height, int width);
void labelImage(bool** binaryImage, int** labelImage, int height, int width);

#endif