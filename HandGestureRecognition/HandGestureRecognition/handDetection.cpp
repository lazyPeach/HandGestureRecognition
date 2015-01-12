#include "stdafx.h"
#include "handDetection.h"
#include "imageProcessing.h"

#include <map>
#include <list>
#include <algorithm>

using namespace cv;
using namespace std;

extern ComponentsMap componentsMap;

vector<HullPoint> handPoints;
vector<HullPoint> hullPoints;

list<Point> fingerPoints;
list<Point> defectPoints;


void createVectorOfHandPoints(int maxAreaLabel, int** labeledImage, int height, int width);
BigInt crossProduct(const HullPoint &O, const HullPoint &A, const HullPoint &B);
void getPointWithMaxHeightBetween(bool** binaryImage, Point p1, Point p2, Point& resPt, float& height, int imgHeight, int imgWidth);
void getFingerPoints(list<Point> tempPts);


Point findCenterPoint(int maxAreaLabel, int** labeledImage, int height, int width) {
  double xc=0, yc=0;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (labeledImage[i][j] == maxAreaLabel) {
        xc += j;
        yc += i;
      }
    }
  }

  xc /= componentsMap[maxAreaLabel].area;
  yc /= componentsMap[maxAreaLabel].area;
  Point returnPt( (int)xc, (int)yc );

  return returnPt;
}

// Computes the convex hull for the object that has the maximum area.
void convexHull(int maxAreaLabel, int** labeledImage, int height, int width) {
  createVectorOfHandPoints(maxAreaLabel, labeledImage, height, width);

  int n = (int)handPoints.size(), k = 0;
  hullPoints.clear();
  hullPoints.resize(2*n);
	
  sort(handPoints.begin(), handPoints.end());
	
  // Build lower hull
	for (int i = 0; i < n; i++) {
    while (k >= 2 && crossProduct(hullPoints[k-2], hullPoints[k-1], handPoints[i]) <= 0) k--;
    handPoints[i].up = false;
    hullPoints[k++] = handPoints[i];
	}

	// Build upper hull
	for (int i = n-2, t = k+1; i >= 0; i--) {
    while (k >= t && crossProduct(hullPoints[k-2], hullPoints[k-1], handPoints[i]) <= 0) k--;
    handPoints[i].up = true;
    hullPoints[k++] = handPoints[i];
	}

  hullPoints.pop_back();// last element is duplicated
  hullPoints.resize(k);
}

void createVectorOfHandPoints(int maxAreaLabel, int** labeledImage, int height, int width) {
  handPoints.clear();
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < height; j++) {
      if (labeledImage[i][j] == maxAreaLabel) {
        handPoints.push_back(HullPoint(j,i));
      }
    }
  }

  int x = 0; 
}

// Get points from convex hull 2 by 2 and make a contour pass from one to another. For each point
// on the contour compute the distance from it to the line described by the two points. If the 
// distance is bigger than the HEIGHT_THRESHOLD keep that point in a list of defect points and add
// the two points in the convex hull to a temporary list.
// Add the first point in the temporary list to the list of points corresponding to fingers. Then
// pass the temporary list and compute the distance between last point in the fingers list and the
// current point. If the distance is bigger than DISTANCE_THRESHOLD add that point to the final list
// of fingers.
void constructResult(bool** binaryImage, int imgHeight, int imgWidth) {
  fingerPoints.clear();
  defectPoints.clear();
  
  list<Point> tempList;
  for (auto it = hullPoints.begin(); it != hullPoints.end()-1; ++it) {
    Point distPt;
    float height = 0;

    Point p1(it->x, it->y);
    Point p2((it+1)->x, (it+1)->y);

    getPointWithMaxHeightBetween(binaryImage, p1, p2, distPt, height, imgHeight, imgWidth);

    if (height > HEIGHT_THRESHOLD) {
      tempList.push_back(p1);
      tempList.push_back(p2);
      defectPoints.push_back(distPt);
    }
  }

  getFingerPoints(tempList);
}


// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
BigInt crossProduct(const HullPoint &O, const HullPoint &A, const HullPoint &B) {
	return (A.x - O.x) * (BigInt)(B.y - O.y) - (A.y - O.y) * (BigInt)(B.x - O.x);
}

// Go from point p1 to p2 on contour and compute the height for each point. Return the point that
// corresponds to the maximum height and its height.
void getPointWithMaxHeightBetween(bool** binaryImage, Point p1, Point p2, Point& resPt, float& height, int imgHeight, int imgWidth) {
  int dx[] = {1,1,0,-1,-1,-1,0,1};
  int dy[] = {0,-1,-1,-1,0,1,1,1};
  int dir = 7;

  for (int i = 0; i < imgWidth; ++i) {
    binaryImage[0][i] = false;
    binaryImage[imgHeight-1][i] = false;
  }

  for (int i = 0; i < imgHeight; ++i) {
    binaryImage[i][0] = false;
    binaryImage[i][imgWidth-1] = false;
  }

  Point crtPoint = p1;
  vector<Point> points;
  
  // get the line between the two points
  float a = p1.y - p2.y;
  float b = p2.x - p1.x;
  float c = p1.x*p2.y - p2.x*p1.y;

  int flag = true;

  while(true){
    //compute the position to start searching for the next point
    if (dir % 2 == 0)
      dir = (dir + 7) % 8;
    else
      dir = (dir + 6) % 8;

    //go through all neighbors until you find a black one
    int y = crtPoint.y + dy[dir];
    int x = crtPoint.x + dx[dir];
    while( binaryImage[y][x] ) {
      dir = (dir + 1) % 8 ;
      y = crtPoint.y + dy[dir];
      x = crtPoint.x + dx[dir];
      if (y <= 0 || y >= imgHeight || x <= 0 || x >= imgWidth) {
        flag = false;
        break;
      }
    }

    if (!flag) break;

    //take the next point
    crtPoint.x = crtPoint.x + dx[dir];
    crtPoint.y = crtPoint.y + dy[dir];
    binaryImage[crtPoint.y][crtPoint.x] = 0;
    points.push_back(crtPoint);

    // for each point check the height
    float f = abs(a * crtPoint.x + b * crtPoint.y + c);
    float d = sqrt(a * a + b * b);
    float dist = f / d;
    if (dist > height) {
      height = dist;
      resPt = crtPoint;
    }

    //check if current point is in the neighbourhood of p2 since p2 is no necessarily on the contour
    if ( abs(crtPoint.x - p2.x) < 2  && abs(crtPoint.y - p2.y) < 2) {
      int x = 0;
      break;
    }

    /*if ( (crtPoint.x <= p2.x+2 && crtPoint.x > p2.x - 2 ) && (crtPoint.y <= p2.y + 2 && crtPoint.y > p2.y - 2)) {
      int x = 0;
      break;
    }*/
  }
}

void getFingerPoints(list<Point> tempPts) {
  fingerPoints.push_back(tempPts.front());

  // go through all points and check if they are close to the last point in the finger list or not
  for (auto it = tempPts.begin(); it != tempPts.end(); ++it) {
    // check distance from new point to the last point in the list
    float distance = sqrt( (it->x - fingerPoints.back().x) * (it->x - fingerPoints.back().x)
      + (it->y - fingerPoints.back().y) * (it->y - fingerPoints.back().y) );

    if ( distance > DISTANCE_THRESHOLD ) {
      fingerPoints.push_back(*it);
    }
  }
}
