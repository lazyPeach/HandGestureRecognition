#include "stdafx.h"

#include <queue>
#include <map>
#include <iostream>
#include <opencv2/highgui/highgui.hpp>

#include "imageProcessing.h"

using namespace std;
using namespace cv;


typedef map<int, Component> ComponentsMap;

ComponentsMap componentsMap;
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
int labelEntireObjectAndGetArea(bool** binaryImage, int** labelImage, Point startPoint, int label) {
  int area = 0;

  queue<Point> pointsQueue;
  pointsQueue.push(startPoint);
  
  while( !pointsQueue.empty() ) {
    Point pt = pointsQueue.front();
    pointsQueue.pop();
    checkNeighbour(binaryImage, labelImage, pt.x-1, pt.y, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x+1, pt.y, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x, pt.y+1, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x, pt.y-1, pointsQueue, label);
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
        int area = labelEntireObjectAndGetArea(binaryImage, labelImage, startPoint, label);

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

