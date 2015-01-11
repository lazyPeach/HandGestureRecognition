#include "stdafx.h"

#include <queue>
#include <map>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <opencv2/highgui/highgui.hpp>

#include "imageProcessing.h"

using namespace std;
using namespace cv;


typedef map<int, Component> ComponentsMap;

ComponentsMap componentsMap;
vector<int> verticalProjectionVector;
bool** aux;
vector<HullPoint> handPoints;
vector<HullPoint> hullPoints;
list<Point> contourList;
list<Point> fingerPoints;
list<Point> defectPoints;


void initializeAux(bool** image, int height, int width) {
  for (int i = 1; i < height; i++) {
    for (int j = 1; j < width; j++) {
      aux[i][j] = image[i][j];
    }
  }
}

void putResultInInitialImg(bool** image, int height, int width) {
  for (int i = 1; i < height; i++) {
    for (int j = 1; j < width; j++) {
      image[i][j] = aux[i][j];
    }
  }
}

void erodeImage(bool** image, int height, int width) {
  initializeAux(image, height, width);

  for (int i = 1; i < height-1; i++) {
    for (int j = 1; j < width-1; j++) {
      if (image[i][j]) {
        /*aux[i][j] = image[i][j-1] && image[i][j+1] && 
                image[i-1][j] && image[i+1][j];
        */
        aux[i][j] = image[i][j-1] && image[i][j+1] && 
          image[i-1][j] && image[i-1][j-1] && image[i-1][j+1] && 
          image[i+1][j] && image[i+1][j-1] && image[i+1][j+1];
      }
    }
  }

  putResultInInitialImg(image, height, width);
}

void dilateImage(bool** image, int height, int width) {
  initializeAux(image, height, width);

  for (int i = 1; i < height-1; i++) {
    for (int j = 1; j < width-1; j++) {
      if (image[i][j]) {
        aux[i][j-1] = true;aux[i][j+1] = true; 
        aux[i-1][j] = true;aux[i-1][j-1] = true;aux[i-1][j+1] = true; 
        aux[i+1][j] = true;aux[i+1][j-1] = true;aux[i+1][j+1] = true;

      }
    }
  }

  putResultInInitialImg(image, height, width);
}

void openImage(bool** image, int height, int width) {
  dilateImage(image, height, width);
  erodeImage(image, height, width);

}

void closeImage(bool** image, int height, int width) {
  aux = new bool*[height];
  for (int i = 0; i < height; i++)
    aux[i] = new bool[width];

  for (int i = 0; i < 2; i++)
    erodeImage(image, height, width);

  for (int i = 0; i < 3; i++)
    dilateImage(image, height, width);

  for (int i = 1; i < height-1; i++) 
    delete aux[i];
  delete aux;
}

void initializeLabelImage(int** labelImage, int height, int width) {
  for (int i = 1; i < height-1; i++) 
    for (int j = 1; j < width-1; j++) 
      labelImage[i][j] = 0;
}

// checks if a point is not visisted, applies a label and pushes that point 
void checkNeighbour(bool** binaryImage, int** labelImage, int xCoord, int yCoord, queue<Point>& pointsQueue, int label) {
  if ( binaryImage[yCoord][xCoord] && labelImage[yCoord][xCoord] == 0 ) {
    labelImage[yCoord][xCoord] = label;
    Point newPt(xCoord, yCoord);
    pointsQueue.push(newPt);
  }
}

// While labeling calculate the area
int labelEntireObjectAndGetArea(bool** binaryImage, int** labelImage, Point startPoint, int label, int height, int width) {
  int area = 0;

  queue<Point> pointsQueue;
  pointsQueue.push(startPoint);

  while( !pointsQueue.empty() ) {
    Point pt = pointsQueue.front();
    pointsQueue.pop();

    if ( (pt.x > 1 && pt.x < width-1) && (pt.y > 1 && pt.y < height-1) ) {

      checkNeighbour(binaryImage, labelImage, pt.x-1, pt.y, pointsQueue, label);
      checkNeighbour(binaryImage, labelImage, pt.x+1, pt.y, pointsQueue, label);
      checkNeighbour(binaryImage, labelImage, pt.x, pt.y+1, pointsQueue, label);// if I comment this line release works as well
      checkNeighbour(binaryImage, labelImage, pt.x, pt.y-1, pointsQueue, label);

    }
    area++;
  }

  return area;
}

void addElementToMap(int label, Point startPoint, int area) {
  Component component;
  component.entryPt = startPoint;
  component.area = area;
  componentsMap[label] = component;
}

// Scan the image until you find a point that might correspond to hand.
// Start a BFS to find the entire object and label it.
void labelImage(bool** binaryImage, int** labelImage, int height, int width) {
  initializeLabelImage(labelImage, height, width);

  int label = 0;

  for (int i = 1; i < height-1; i++) {
    for (int j = 1; j < width-1; j++) {
      if (binaryImage[i][j] && (labelImage[i][j] == 0)) {//if not labeled foreground pixel
        label++;
        Point startPoint(j,i);
        labelImage[startPoint.y][startPoint.x] = label;
        int area = labelEntireObjectAndGetArea(binaryImage, labelImage, startPoint, label, height, width);

        addElementToMap(label, startPoint, area);
      } 
    }
  }
}

// returns the label corresponding to the component with the maximum area
int getLabelWithMaxArea() {
  int resultLabel;
  int maxArea = 0;

  for (ComponentsMap::iterator it = componentsMap.begin(); it != componentsMap.end(); it++) {
    if (it->second.area > maxArea) {
      maxArea = it->second.area;
      resultLabel = it->first;
    }
  }

  return resultLabel;
}

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

/*
 *     3 2 1
 *      \|/
 *     4-P-0
 *      /|\
 *     5 6 7
 */

void contourTracing(bool** binaryImage, int height, int width, int maxAreaLabel) {
  Point crtPoint = componentsMap[maxAreaLabel].entryPt;
  int dx[] = {1,1,0,-1,-1,-1,0,1};
  int dy[] = {0,-1,-1,-1,0,1,1,1};
  int dir = 7;

  vector<Point> points;

  while(true){
    //compute the position to start searching for the next point
    if (dir % 2 == 0)
      dir = (dir + 7) % 8;
    else
      dir = (dir + 6) % 8;

    //go through all neighbors until you find a black one
    while( binaryImage[crtPoint.y + dy[dir]][crtPoint.x + dx[dir]] )
      dir = (dir + 1) % 8 ;

    //take the next point
    crtPoint.x = crtPoint.x + dx[dir];
    crtPoint.y = crtPoint.y + dy[dir];
    binaryImage[crtPoint.y][crtPoint.x] = 0;
    points.push_back(crtPoint);

    if(points.size() > 2)
      if ( (crtPoint.x == points.front().x) && (crtPoint.y == points.front().y))//crt point == start point
        break;
  }    

  //remove last element since it is equal to the first
  points.pop_back();

  int x = 0;
  for (auto it = points.begin(); it != points.end(); ++it) {
    Point p(it->x, it->y);
    contourList.push_back(p);
  } 
}

void createVectorOfHandPoints(int maxAreaLabel, int** labeledImage, int height, int width) {
  handPoints.clear();
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < height; j++) {
      if (labeledImage[i][j] == maxAreaLabel) {
        HullPoint p(j,i);
        handPoints.push_back(p);
      }
    }
  }

  int x = 0; 
}


// 2D cross product of OA and OB vectors, i.e. z-component of their 3D cross product.
// Returns a positive value, if OAB makes a counter-clockwise turn,
// negative for clockwise turn, and zero if the points are collinear.
BigInt cross(const HullPoint &O, const HullPoint &A, const HullPoint &B) {
	return (A.x - O.x) * (BigInt)(B.y - O.y) - (A.y - O.y) * (BigInt)(B.x - O.x);
}

// not usefull - maybe for future improvement???
cv::Point getConvexDefect(bool** binaryImage, Point startPoint, Point endPoint, Point centerPoint) {
  Point result = startPoint;
  int dx[] = {1,1,0,-1,-1,-1,0,1};
  int dy[] = {0,-1,-1,-1,0,1,1,1};

  if (centerPoint.y > startPoint.y && centerPoint.y > endPoint.y) { // center point is below -> find maxY
    //contour tracing fromstart to end
    Point crtPoint = startPoint;

    int dir = 7;

    if (startPoint != endPoint)

    while(true){
      //compute the position to start searching for the next point
      if (dir % 2 == 0)
        dir = (dir + 7) % 8;
      else
        dir = (dir + 6) % 8;

      //go through all neighbors until you find a black one
      while( binaryImage[crtPoint.y + dy[dir]][crtPoint.x + dx[dir]] )
        dir = (dir + 1) % 8 ;

      //take the next point
      crtPoint.x = crtPoint.x + dx[dir];
      crtPoint.y = crtPoint.y + dy[dir];
      binaryImage[crtPoint.y][crtPoint.x] = 0;

      if (crtPoint.y > result.y) result = crtPoint;
      
      if ( (crtPoint.x == endPoint.x) && (crtPoint.y == endPoint.y)) {
        break;
      }
    }

  }

  int x = 0;

  return result;
}

void convexHull() {
  int n = (int)handPoints.size(), k = 0;
  hullPoints.clear();
  hullPoints.resize(2*n);
	
  // Sort points lexicographically
  sort(handPoints.begin(), handPoints.end());
  int x = 0;
	
  // Build lower hull
	for (int i = 0; i < n; i++) {
    while (k >= 2 && cross(hullPoints[k-2], hullPoints[k-1], handPoints[i]) <= 0) k--;
    handPoints[i].up = false;
    hullPoints[k++] = handPoints[i];
	}

	// Build upper hull
	for (int i = n-2, t = k+1; i >= 0; i--) {
    while (k >= t && cross(hullPoints[k-2], hullPoints[k-1], handPoints[i]) <= 0) k--;
    handPoints[i].up = true;
    hullPoints[k++] = handPoints[i];
	}

  hullPoints.pop_back();// last element is duplicated

  hullPoints.resize(k);
}

void getPointWithMaxHeightBetween(bool** binaryImage, Point p1, Point p2, Point& resPt, float& height) {
  int dx[] = {1,1,0,-1,-1,-1,0,1};
  int dy[] = {0,-1,-1,-1,0,1,1,1};
  int dir = 7;

  Point crtPoint = p1;

  vector<Point> points;

  

  // get the line between the two points
  float a = p1.y - p2.y;
  float b = p2.x - p1.x;
  float c = p1.x*p2.y - p2.x*p1.y;

  while(true){
    //compute the position to start searching for the next point
    if (dir % 2 == 0)
      dir = (dir + 7) % 8;
    else
      dir = (dir + 6) % 8;

    //go through all neighbors until you find a black one
    while( binaryImage[crtPoint.y + dy[dir]][crtPoint.x + dx[dir]] )
      dir = (dir + 1) % 8 ;

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



    //verifica daca si x si y sunt pre aproape
    if ( (crtPoint.x <= p2.x+2 && crtPoint.x > p2.x - 2 ) && (crtPoint.y <= p2.y + 2 && crtPoint.y > p2.y - 2)) {
      int x = 0;
      break;
    }
  }    

}

// this can be improved
void constructResult(bool** binaryImage) {
  // take points from convex hull 2 by 2
  // first find two points that could resemble two fingers
  list<Point> tempList;
  vector<HullPoint>::iterator it = hullPoints.begin();
  Point p(it->x, it->y);

  fingerPoints.clear();

  for (auto it = hullPoints.begin(); it != hullPoints.end()-1; ++it) {
    Point distPt;
    float height = 0;

    Point p1(it->x, it->y);
    Point p2((it+1)->x, (it+1)->y);

    getPointWithMaxHeightBetween(binaryImage, p1, p2, distPt, height);

    if (height > 30) {
      tempList.push_back(p1);
      tempList.push_back(p2);
      defectPoints.push_back(distPt);
    }



  }

  int x = 0;

  //consider first point as being a finger and insert it into final list
  fingerPoints.push_back(tempList.front());

  // go through all points and check if they are close to the last point in the finger list or not
  for (auto it = tempList.begin(); it != tempList.end(); ++it) {
    // check distance from new point to the last point in the list
    float distance = sqrt( (it->x - fingerPoints.back().x) * (it->x - fingerPoints.back().x)
      + (it->y - fingerPoints.back().y) * (it->y - fingerPoints.back().y) );

    if ( distance > 15 ) {
      fingerPoints.push_back(*it);
    }
  }

  int y = 0;

}
