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
//extern vector<HullPoint> hullPoints;
extern list<Point> lista;
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

void drawFilledCircle(Mat& image, Point center, int thickness) {
 circle( image,
         center,
         thickness,
         Scalar( 0, 0, 255 ),
         CV_FILLED,
         8 );
}

int _tmain(int argc, _TCHAR* argv[]) {
  //namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  namedWindow("Result",CV_WINDOW_AUTOSIZE);
  int mouseParam= CV_EVENT_FLAG_LBUTTON;
  //cvSetMouseCallback("Initial",mouseHandler,&mouseParam);
  cvSetMouseCallback("Result",mouseHandler,&mouseParam);

  VideoCapture cap(0);

  Mat image;// = imread("../samples/img0.jpg", CV_LOAD_IMAGE_COLOR);

  Mat backgroundImage;// = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);
  cap >> backgroundImage;
  
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

  while(1) {
    cap.read(image);

    //if(! image.data ) { // Check for invalid input
    //  cout << "Could not open or find the image" << std::endl ;
    //  return -1;
    //}

    //Mat result(image.rows, image.cols, image.type());
    
    

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
    createVectorOfHandPoints(maxAreaLabel, labeledImage, height, width);//take care... this is 
    convexHull();
    constructResult();
    
    /*
    //put the binarization matrix in result
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b color;

        //if ( binaryImage[i][j] ) {
          color[0] = imageRGB[i][j].blue;
          color[1] = imageRGB[i][j].green;
          color[2] = imageRGB[i][j].red;
      
        result.at<Vec3b>(Point(j,i)) = color;
      }
    }
    */

    //draw into image the center pt
    drawFilledCircle(image, centerPoint, 5);

    for (list<Point>::iterator it = lista.begin(); it != lista.end(); it++) {
      circle( image, *it, 2, Scalar( 255, 0, 0 ), CV_FILLED, 8 );
      circle( image, *it, 15, Scalar( 255, 255, 0 ), 1, 8 );

    }

    t = clock() - t;
    cout << "Processing time = " << t << " miliseconds" << endl;
    cout << "Processing time = " << t/(float)CLOCKS_PER_SEC << " seconds" << endl;

    
   

    //imshow( "Initial", backgroundImage);//image );
    imshow("Result", image);
    image.release();
    
    if (waitKey(30) == 27){                             //wait for 'esc' key press for 30ms. If 'esc' key is pressed, break loop
      cout << "esc key is pressed by user" << endl;
      break;
    }
  }
  
  return 0;
}

