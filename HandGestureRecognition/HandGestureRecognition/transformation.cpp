#include "stdafx.h"
#include "transformation.h"

using namespace cv;


double  getValue(double red, double green, double blue);
double  getMinRGB(double red, double green, double blue);
double  getSaturation(double red, double green, double blue, double value);
double  getHue(double red, double green, double blue, double value, double saturation);
bool    isHueInBounds(double hue);
bool    isSaturationInBounds(double saturation);

void transformMatToRGBMatrix(Mat image, colorRGB** matrixRGB) {
  for(int i = 0; i < image.rows; i++) {
    for(int j = 0; j < image.cols; j++) {
      Vec3b color = image.at<Vec3b>(Point(j,i));
      
      matrixRGB[i][j].red = color[2];
      matrixRGB[i][j].green = color[1];
      matrixRGB[i][j].blue = color[0];
    }
  }
}

void transformRGBToHSV(colorRGB** imageRGB, colorHSV** imageHSV, int height, int width) {
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      imageHSV[i][j].value = getValue(imageRGB[i][j].red, 
                                      imageRGB[i][j].green, 
                                      imageRGB[i][j].blue);
      imageHSV[i][j].saturation = getSaturation(imageRGB[i][j].red,
                                                imageRGB[i][j].green,
                                                imageRGB[i][j].blue,
                                                imageHSV[i][j].value); 
      imageHSV[i][j].hue = getHue(imageRGB[i][j].red,
                                  imageRGB[i][j].green,
                                  imageRGB[i][j].blue,
                                  imageHSV[i][j].value,
                                  imageHSV[i][j].saturation);
    }
  }
}

void binarizeHSVImage(colorHSV** imageHSV, bool** binaryImage, int height, int width) {
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      binaryImage[i][j] = isHueInBounds(imageHSV[i][j].hue) && 
                          isSaturationInBounds(imageHSV[i][j].saturation);
    }
  }
}

double getValue(double red, double green, double blue) {
  return ( (red >= green) && (red >= blue) ) ? red : 
                ( green >= blue ) ? green : blue;
}

double getMinRGB(double red, double green, double blue) {
  return ( (red <= green) && (red <= blue) ) ? red :
         ( green <= blue ) ? green : blue;
}

double getSaturation(double red, double green, double blue, double value) {
  if ( value == 0 )
    return 0;
  
  double min = getMinRGB( red, green, blue );
  return (value - min) / value;
}

double getHue(double red, double green, double blue, double value, double saturation) {
  if ( saturation == 0 )
    return -1;
  
  double min = getMinRGB( red, green, blue );
  double Cr = (value - red) / (value - min);
  double Cg = (value - green) / (value - min);
  double Cb = (value - blue) / (value- min);

  double ret = ( red == value ) ? (Cb - Cg) :
               ( green == value ) ? ( 2 + Cr - Cb ) : ( 4 + Cg - Cr );
  ret *= 60;
  ret += (ret < 0) ? 360 : 0;
  
  return ret;
}

// todo define bounds in .h file
bool isHueInBounds(double hue) {
  return ( (hue > 0 && hue < 30) || (hue > 300 && hue < 360) );
}

bool isSaturationInBounds(double saturation) {
  return ( saturation > 0.15 && saturation < 0.9 );
}
