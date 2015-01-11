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

using namespace cv;
using namespace std;

extern map<int, Component> componentsMap;
extern vector<Point> extremities;
extern vector<HullPoint> hullPoints;
extern vector<HullPoint> handPoints;
extern list<Point> contourList;
extern list<Point> defectPoints;
extern list<Point> fingerPoints;


colorRGB** imageRGB;
colorRGB** backgroundRGB;
colorHSV** imageHSV;
bool** binaryImage;
int** labeledImage;

void mouseHandler(int event, int y, int x, int flags, void* param) {
  switch( event ) {
  case CV_EVENT_LBUTTONDOWN:
    cout << endl;
    cout << "mouse at position: " << x << " " << y << endl;
    cout << "Label: " << labeledImage[x][y] << endl;
    cout << "HSV: " << imageHSV[x][y].hue << " " << imageHSV[x][y].saturation <<endl;
    break;
  default:
    break;
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
      imageRGB[i][j].red = imageRGB[i][j].red - backgroundRGB[i][j].red;
      imageRGB[i][j].green = imageRGB[i][j].green - backgroundRGB[i][j].green;
      imageRGB[i][j].blue = imageRGB[i][j].blue - backgroundRGB[i][j].blue;
    }
  }
}

void drawConvexHull(Mat& image) {
  for (auto it = hullPoints.begin(); it != hullPoints.end()-1; ++it) {
    line(image, Point(it->x, it->y), Point((it+1)->x, (it+1)->y), Scalar(255, 255, 0), 1); 
  }


}



int _tmain(int argc, _TCHAR* argv[]) {
  namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  //namedWindow("Result",CV_WINDOW_AUTOSIZE);
  int mouseParam= CV_EVENT_FLAG_LBUTTON;
  cvSetMouseCallback("Initial",mouseHandler,&mouseParam);
  //cvSetMouseCallback("Result",mouseHandler,&mouseParam);

  Mat image = imread("../samples/img8.jpg", CV_LOAD_IMAGE_COLOR);
  Mat backgroundImage = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);



  //memory allocation region
  int height = backgroundImage.rows;
  int width = backgroundImage.cols;

  imageRGB = new colorRGB*[height];
  backgroundRGB = new colorRGB*[height];
  imageHSV = new colorHSV*[height];
  binaryImage = new bool*[height];
  labeledImage = new int*[height];

  for ( int i = 0; i < height; i++ ) {
    imageRGB[i] = new colorRGB[width];
    backgroundRGB[i] = new colorRGB[width];
    imageHSV[i] = new colorHSV[width];
    binaryImage[i] = new bool[width];
    labeledImage[i] = new int[width];
  }

  setBackground(backgroundImage);


  Mat testResult(image.rows, image.cols, image.type());


  transformMatToRGBMatrix(image, imageRGB);
  subtractBackground(height, width);
  transformRGBToHSV(imageRGB, imageHSV, height, width);
  binarizeHSVImage(imageHSV, binaryImage, height, width);
  closeImage(binaryImage, height, width);
  labelImage(binaryImage, labeledImage, height, width);
  int maxAreaLabel = getLabelWithMaxArea();
  Point centerPoint = findCenterPoint(maxAreaLabel, labeledImage, height, width);
  createVectorOfHandPoints(maxAreaLabel, labeledImage, height, width);//take care... this is 
  convexHull();
  //contourTracing(binaryImage, height, width, maxAreaLabel);
  constructResult(binaryImage);

  int x = 0;

  //put the binarization matrix in result
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      Vec3b color;

      if ( binaryImage[i][j] ) {
        color[0] = imageRGB[i][j].blue;
        color[1] = imageRGB[i][j].green;
        color[2] = imageRGB[i][j].red;
      } else {
        color[0] = 0;
        color[1] = 0;
        color[2] = 0;
      }

      testResult.at<Vec3b>(Point(j,i)) = color;

    }
  }

  drawConvexHull(image);
  
  circle( image, centerPoint, 5, Scalar( 0, 0, 255 ), CV_FILLED, 8 );

  for (auto it = fingerPoints.begin(); it != fingerPoints.end(); it++) {
    circle( image, *it, 2, Scalar( 0, 255, 255 ), CV_FILLED);
    circle( image, *it, 10, Scalar( 0, 255, 255 ));
  }


  for (auto it = defectPoints.begin(); it != defectPoints.end(); ++it) {
    circle( image, *it, 7, Scalar( 255, 0, 255 ));
  }

  imshow( "Initial", image );
  //imshow("Result", testResult);

  waitKey(0);

  return 0;
}


