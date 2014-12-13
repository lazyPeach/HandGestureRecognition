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
colorHSV** imageHSV;
bool** binaryImage;

int _tmain(int argc, _TCHAR* argv[]) {
  
  Mat image = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);

  if(! image.data ) { // Check for invalid input
    cout << "Could not open or find the image" << std::endl ;
    return -1;
  }
  
  Mat result(image.rows, image.cols, image.type());

  namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  namedWindow("Result",CV_WINDOW_AUTOSIZE);
  
  int height = image.rows;
  int width = image.cols;
  
  imageRGB = new colorRGB*[height];
  imageHSV = new colorHSV*[height];
  binaryImage = new bool*[height];

  for ( int i = 0; i < height; i++ ) {
    imageRGB[i] = new colorRGB[width];
    imageHSV[i] = new colorHSV[width];
    binaryImage[i] = new bool[width];
  }

  clock_t t = clock();

  transformMatToRGBMatrix(image, imageRGB);
  transformRGBToHSV(imageRGB, imageHSV, height, width);
  binarizeHSVImage(imageHSV, binaryImage, height, width);
  closeImage(binaryImage, height, width);
  

  t = clock() - t;
  cout << "Processing time = " << t << " miliseconds" << endl;
  cout << "Processing time = " << t/(float)CLOCKS_PER_SEC << " seconds" << endl;


  //put the binarization matrix in result
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      Vec3b color;

      
      if ( binaryImage[i][j] ) {
        color[0] = 0;//imageRGB[i][j].blue;
        color[1] = 150;//imageRGB[i][j].green;
        color[2] = 100;//imageRGB[i][j].red;
      } else {
        color[0] = 255;
        color[1] = 255;
        color[2] = 255;
      }
      result.at<Vec3b>(Point(j,i)) = color;
    }
  }
  
  imshow( "Initial", image );
  imshow("Result", result);
  
  waitKey(0); // Wait for a keystroke in the window
	return 0;
}

