#ifndef _IMAGE_PROCESSING_H_
#define _IMAGE_PROCESSING_H_

//a structure that holds the 
typedef struct _Component {
  cv::Point entryPt;
  int area;
} Component;

// structure for points used in finding the convex hull
typedef struct _HullPoint {
  int x, y;
  bool up;

  _HullPoint() {
    x = 0; y = 0;
  }

  _HullPoint(int x, int y) {
    this->x = x; this->y = y;
  }

  _HullPoint(const cv::Point p) {
    x = p.x; y = p.y;
  }

  bool operator < (const _HullPoint& p) const {
    return x < p.x || (x == p.x && y < p.y);
  }
} HullPoint;

typedef long long BigInt;


void openImage(bool** image, int height, int width);
void closeImage(bool** image, int height, int width);
void labelImage(bool** binaryImage, int** labelImage, int height, int width);
int getLabelWithMaxArea();
cv::Point findCenterPoint(int maxAreaLabel, int** labeledImage, int height, int width);
void contourTracing(bool** binaryImage, int height, int width, int maxAreaLabel);
void createVectorOfHandPoints(int maxAreaLabel, int** labeledImage, int height, int width);


void convexHull();

#endif