#ifndef _HAND_DETECTION_H_
#define _HAND_DETECTION_H_

#include <opencv2\core\core.hpp>

#define HEIGHT_THRESHOLD 30
#define DISTANCE_THRESHOLD 15

// structure used in finding the convex hull
typedef struct _HullPoint {
  int x, y;
  bool up;

  _HullPoint() {
    x = 0; y = 0;
  }

  _HullPoint(int x, int y) {
    this->x = x; this->y = y;
  }
  
  bool operator < (const _HullPoint& p) const {
    return x < p.x || (x == p.x && y < p.y);
  }
} HullPoint;

typedef long long BigInt;

// Computes the center point of the object having the biggest area.
cv::Point findCenterPoint(int maxAreaLabel, int** labeledImage, int height, int width);
// Computes the convex hull of the object having the biggest area.
void convexHull(int maxAreaLabel, int** labeledImage, int height, int width);
// Constructs the result consisting of finger points and convexity defects
void constructResult(bool** binaryImage, int imgHeight, int imgWidth);

#endif
