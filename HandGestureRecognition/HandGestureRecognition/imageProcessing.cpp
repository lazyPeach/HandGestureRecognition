#include "stdafx.h"

#include <queue>
#include <map>
#include <vector>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "imageProcessing.h"

using namespace std;
using namespace cv;


typedef map<int, Component> ComponentsMap;

ComponentsMap componentsMap;
vector<Point> extremities;
bool** aux;

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
          image[i-1][j] && image[i+1][j];*/
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
  double cc=0, rc=0;
  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      if (labeledImage[i][j] == maxAreaLabel) {
        cc += j;
        rc += i;
      }
    }
  }

  cc /= componentsMap[maxAreaLabel].area;
  rc /= componentsMap[maxAreaLabel].area;
  Point returnPt( (int)cc, (int)rc );

  return returnPt;
}

void findExtremities(vector<Point> pointsList) {
  //prepare vector
  for (int i = 2; i < pointsList.size()-1; i++) {
    
    //comp with prev and after
    if ( pointsList[i].y >= pointsList[i-1].y
      && pointsList[i].y >= pointsList[i-2].y
      && pointsList[i].y >= pointsList[i+1].y
      && pointsList[i].y >= pointsList[i+2].y
        ) {
          //cout<< "found local maxima " << pointsList[i].x << " " << pointsList[i].y << endl; 
          extremities.push_back(pointsList[i]);
    }

  }

  //for (int i = 0; i < )
  
  /*list<Point>::iterator it = pointsList.begin();
  it++; it++;*/
  //Point pt = pointsList[pointsList.size()-1];
 //int x = 0;
/*
  for ( ; it != pointsList.end(); it++) {
    




  }
*/


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

    //if (dir == 1) {
    //  cout << "found an up at " << crtPoint.x + dx[dir] << " " << crtPoint.y + dy[dir] << endl;
    //}

    //take the next point
    crtPoint.x = crtPoint.x + dx[dir];
    crtPoint.y = crtPoint.y + dy[dir];
    binaryImage[crtPoint.y][crtPoint.x] = 0;
    points.push_back(crtPoint);

    if(points.size() > 2)
      if ( (crtPoint.x == points.front().x) && (crtPoint.y == points.front().y))//crt point == start point
        break;
  }	

  // prepare list for parsing
  //remove last element since it is equal to the first
  points.pop_back();
 
  findExtremities(points);
}


