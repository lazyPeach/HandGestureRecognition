#ifndef _TRANFORMATION_H_
#define _TRANFORMATION_H_

#include "defines.h"
#include <opencv2/core/core.hpp>

// Transforms the Mat object into a raw representation of the image (a 2D array).
void transformMatToRGBMatrix(cv::Mat image, colorRGB** matrixRGB);

// Transforms a 2D matrix containing RGB information about the image into a 2D matrix contianing
// HSV information about the image.
void transformRGBToHSV( colorRGB** imageRGB, colorHSV** imageHSV, int height, int width );

// Converts a HSV 2D matrix into a binary matrix which had a black background and white objects.
void binarizeHSVImage(colorHSV** imageHSV, bool** binaryImage, int height, int width);

#endif
