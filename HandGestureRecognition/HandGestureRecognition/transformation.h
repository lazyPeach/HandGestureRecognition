#ifndef _TRANFORMATION_H_
#define _TRANFORMATION_H_

#include "defines.h"
#include <opencv2/core/core.hpp>


void transformMatToRGBMatrix(cv::Mat image, colorRGB** matrixRGB);
void transformRGBToHSV( colorRGB** imageRGB, colorHSV** imageHSV, int height, int width );
void binarizeHSVImage(colorHSV** imageHSV, bool** binaryImage, int height, int width);

#endif