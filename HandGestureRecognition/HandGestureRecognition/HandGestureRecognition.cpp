#include "stdafx.h"

#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <iostream>
#include <Windows.h>
#include <time.h>

#include "defines.h"
#include "transformation.h"
#include "imageProcessing.h"

using namespace cv;
using namespace std;

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

        break;
    default:
        break;
    }
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

void drawCenterFilledCircle(Mat& image, Point center) {
  int thickness = -1;
 int lineType = 8;

 circle( image,
         center,
         5,
         Scalar( 0, 0, 255 ),
         CV_FILLED,
         lineType );
}

int _tmain(int argc, _TCHAR* argv[]) {
  namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  namedWindow("Result",CV_WINDOW_AUTOSIZE);
  int mouseParam= CV_EVENT_FLAG_LBUTTON;
  cvSetMouseCallback("Initial",mouseHandler,&mouseParam);
  cvSetMouseCallback("Result",mouseHandler,&mouseParam);

  Mat backgroundImage = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);
  Mat image = imread("../samples/img2.jpg", CV_LOAD_IMAGE_COLOR);

  if(! image.data ) { // Check for invalid input
    cout << "Could not open or find the image" << std::endl ;
    return -1;
  }
  
  Mat result(image.rows, image.cols, image.type());


  
  int height = image.rows;
  int width = image.cols;
  
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

  transformMatToRGBMatrix(backgroundImage, backgroundRGB);//doesn't count as computation since it is done only once

  clock_t t = clock();

  transformMatToRGBMatrix(image, imageRGB);
  subtractBackground(height, width);
  transformRGBToHSV(imageRGB, imageHSV, height, width);
  binarizeHSVImage(imageHSV, binaryImage, height, width);
  closeImage(binaryImage, height, width);
  labelImage(binaryImage, labeledImage, height, width);
  int maxAreaLabel = getLabelWithMaxArea();
  Point centerPoint = findCenterPoint(maxAreaLabel, labeledImage, height, width);


  t = clock() - t;
  cout << "Processing time = " << t << " miliseconds" << endl;
  cout << "Processing time = " << t/(float)CLOCKS_PER_SEC << " seconds" << endl;


  //put the binarization matrix in result
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      Vec3b color;

      color[0] = labeledImage[i][j] * 10 % 255;
      color[1] = labeledImage[i][j] * 10 % 255;
      color[2] = labeledImage[i][j] * 10 % 255;
      
      //if ( binaryImage[i][j] ) {
      //  color[0] = 0;//imageRGB[i][j].blue;
      //  color[1] = 150;//imageRGB[i][j].green;
      //  color[2] = 100;//imageRGB[i][j].red;
      //} else {
      //  color[0] = 255;
      //  color[1] = 255;
      //  color[2] = 255;
      //}
      result.at<Vec3b>(Point(j,i)) = color;
    }
  }

  //draw into image the center pt
  //Point center(centerPoint.y, centerPoint.x);
  drawCenterFilledCircle(image, centerPoint);

  
  imshow( "Initial", image );
  imshow("Result", result);
  
  waitKey(0); // Wait for a keystroke in the window
	return 0;
}

