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
  int xCenter;
  int yCenter;
} Component;


void openImage(bool** image, int height, int width);
void closeImage(bool** image, int height, int width);
void labelImage(bool** binaryImage, int** labelImage, int height, int width);
int getLabelWithMaxArea();
Point2D findCenterPoint(int maxAreaLabel, int** labeledImage, int height, int width);

#endif