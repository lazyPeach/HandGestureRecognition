#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

#include <opencv2/core/core.hpp>
#include <map>


//a structure that holds the entry point for an object and its area
typedef struct _Component {
  cv::Point entryPt;
  int area;
} Component;

typedef std::map<int, Component> ComponentsMap;


// Performs image close
void  closeImage(bool** image, int height, int width);
// Assigns a label to each object in the image
void  labelImage(bool** binaryImage, int** labelImage, int height, int width);
// Returns the label corresponding to the object with the maximum area
int   getLabelWithMaxArea();

#endif
