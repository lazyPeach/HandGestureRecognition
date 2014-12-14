#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

//a structure that holds the 
typedef struct _Component {
  cv::Point entryPt;
  //int xEntry;
  //int yEntry;
  int area;
} Component;


void openImage(bool** image, int height, int width);
void closeImage(bool** image, int height, int width);
void labelImage(bool** binaryImage, int** labelImage, int height, int width);
int getLabelWithMaxArea();
cv::Point findCenterPoint(int maxAreaLabel, int** labeledImage, int height, int width);
void contourTracing(bool** binaryImage, int height, int width, int maxAreaLabel);

#endif