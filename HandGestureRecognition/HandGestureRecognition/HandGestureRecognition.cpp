#include "stdafx.h"
#include <opencv2/highgui/highgui.hpp>
#include <math.h>
#include <iostream>
#include <Windows.h>
#include <time.h>

using namespace cv;
using namespace std;

int _tmain(int argc, _TCHAR* argv[]) {
  
  Mat image = imread("../samples/background.jpg", CV_LOAD_IMAGE_COLOR);

  if(! image.data ) { // Check for invalid input
    cout << "Could not open or find the image" << std::endl ;
    return -1;
  }
  
  Mat result(image.rows, image.cols, image.type());
  result.data = image.data;

  namedWindow("Initial",CV_WINDOW_AUTOSIZE);
  namedWindow("Result",CV_WINDOW_AUTOSIZE);
  
  
  imshow( "Initial", image );
  imshow("Result", result);
  
  waitKey(0); // Wait for a keystroke in the window
	return 0;
}

