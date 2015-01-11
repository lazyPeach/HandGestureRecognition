#include "stdafx.h"

#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <list>

#include "defines.h"
#include "transformation.h"
#include "imageProcessing.h"
#include "handDetection.h"

using namespace cv;
using namespace std;

extern vector<HullPoint> hullPoints;
extern list<Point> defectPoints;
extern list<Point> fingerPoints;

colorRGB** backgroundRGB;
colorRGB** imageRGB;
colorHSV** imageHSV;
bool** binaryImage;
int** labeledImage;

void mouseHandler(int event, int y, int x, int flags, void* param);
void allocateMemory(int height, int width);



void setBackground(Mat background);
void subtractBackground(int height, int width);
void drawConvexHull(Mat& image);


int _tmain(int argc, _TCHAR* argv[]) {
  namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  int mouseParam= CV_EVENT_FLAG_LBUTTON;
  cvSetMouseCallback("Initial",mouseHandler,&mouseParam);

  Mat image = imread("../samples/img0.jpg", CV_LOAD_IMAGE_COLOR);
  Mat backgroundImage = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);

  // memory allocation region
  int height = backgroundImage.rows;
  int width = backgroundImage.cols;
  allocateMemory(height, width);

  setBackground(backgroundImage);

  // image transformation region
  transformMatToRGBMatrix(image, imageRGB);
  subtractBackground(height, width);
  transformRGBToHSV(imageRGB, imageHSV, height, width);
  binarizeHSVImage(imageHSV, binaryImage, height, width);
  
  // image processing region
  closeImage(binaryImage, height, width);
  labelImage(binaryImage, labeledImage, height, width);
  int maxAreaLabel = getLabelWithMaxArea();
  
  // hand detection region
  Point centerPoint = findCenterPoint(maxAreaLabel, labeledImage, height, width);
  convexHull(maxAreaLabel, labeledImage, height, width);
  constructResult(binaryImage);
  
  
  circle( image, centerPoint, 5, Scalar( 0, 0, 255 ), CV_FILLED, 8 );
  drawConvexHull(image);
  for (auto it = fingerPoints.begin(); it != fingerPoints.end(); it++) {
    circle( image, *it, 2, Scalar( 0, 255, 255 ), CV_FILLED);
    circle( image, *it, 10, Scalar( 0, 255, 255 ));
  }


  for (auto it = defectPoints.begin(); it != defectPoints.end(); ++it) {
    circle( image, *it, 7, Scalar( 255, 0, 255 ));
  }

  imshow( "Initial", image );

  waitKey(0);

  return 0;
}

// Used mainly for debugging. It gets the mouse position and we can check values of pixels for those positions 
void mouseHandler(int event, int y, int x, int flags, void* param) {
  switch( event ) {
  case CV_EVENT_LBUTTONDOWN:
    cout << endl;
    cout << "mouse at position: " << x << " " << y << endl;
    cout << "HSV: " << imageHSV[x][y].hue << " " << imageHSV[x][y].saturation <<endl;
    break;
  default:
    break;
  }
}


// Allocates memory for the 2D matrices that will be used during the execution of the program
void allocateMemory(int height, int width) {
  imageRGB        = new colorRGB*[height];
  backgroundRGB   = new colorRGB*[height];
  imageHSV        = new colorHSV*[height];
  binaryImage     = new bool*[height];
  labeledImage    = new int*[height];

  for ( int i = 0; i < height; i++ ) {
    imageRGB[i]       = new colorRGB[width];
    backgroundRGB[i]  = new colorRGB[width];
    imageHSV[i]       = new colorHSV[width];
    binaryImage[i]    = new bool[width];
    labeledImage[i]   = new int[width];
  }
}

void setBackground(Mat background) {
  transformMatToRGBMatrix(background, backgroundRGB);
}

void subtractBackground(int height, int width) {
  if (imageRGB == NULL || backgroundRGB == NULL)
    return;

  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      imageRGB[i][j].red    = imageRGB[i][j].red - backgroundRGB[i][j].red;
      imageRGB[i][j].green  = imageRGB[i][j].green - backgroundRGB[i][j].green;
      imageRGB[i][j].blue   = imageRGB[i][j].blue - backgroundRGB[i][j].blue;
    }
  }
}


void drawConvexHull(Mat& image) {
  for (auto it = hullPoints.begin(); it != hullPoints.end()-1; ++it) {
    line(image, Point(it->x, it->y), Point((it+1)->x, (it+1)->y), Scalar(255, 255, 0), 1); 
  }
}
