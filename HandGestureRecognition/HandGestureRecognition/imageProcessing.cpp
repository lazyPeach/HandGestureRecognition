#include "stdafx.h"
#include "imageProcessing.h"

#include <queue>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;


ComponentsMap componentsMap;
bool** aux;


void erodeImage(bool** image, int height, int width);
void dilateImage(bool** image, int height, int width);
void initializeLabelImage(int** labelImage, int height, int width);
int labelEntireObjectAndGetArea(bool** binaryImage, int** labelImage, Point startPoint, int label, int height, int width);
void addElementToMap(int label, Point startPoint, int area);


void closeImage(bool** image, int height, int width) {
  aux = new bool*[height];
  for (int i = 0; i < height; i++)
    aux[i] = new bool[width];

  for (int i = 0; i < 2; i++)
    erodeImage(image, height, width);

  for (int i = 0; i < 2; i++)
    dilateImage(image, height, width);

  for (int i = 1; i < height-1; i++) 
    delete aux[i];
  delete aux;
}

// Scan the image until you find a point that might correspond to hand.
// Start a BFS to find the entire object and label it. During scann compute the area.
void labelImage(bool** binaryImage, int** labelImage, int height, int width) {
  initializeLabelImage(labelImage, height, width);
  int label = 0;

  for (int i = 1; i < height-1; i++) {
    for (int j = 1; j < width-1; j++) {
      if (binaryImage[i][j] && (labelImage[i][j] == 0)) {   //if not labeled foreground pixel
        label++;
        Point startPoint(j,i);
        labelImage[startPoint.y][startPoint.x] = label;
        int area = labelEntireObjectAndGetArea(binaryImage, labelImage, startPoint, label, height, width);

        addElementToMap(label, startPoint, area);
      } 
    }
  }
}

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

// Initializes and auxiliary matrix used in open/close image
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

void initializeLabelImage(int** labelImage, int height, int width) {
  for (int i = 1; i < height-1; i++) 
    for (int j = 1; j < width-1; j++) 
      labelImage[i][j] = 0;
}

// Checks if a neighbour point is not visisted, applies a label and pushes that point 
void checkNeighbour(bool** binaryImage, int** labelImage, int xCoord, int yCoord, queue<Point>& pointsQueue, int label) {
  if ( binaryImage[yCoord][xCoord] && labelImage[yCoord][xCoord] == 0 ) {
    labelImage[yCoord][xCoord] = label;
    Point newPt(xCoord, yCoord);
    pointsQueue.push(newPt);
  }
}

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
      checkNeighbour(binaryImage, labelImage, pt.x, pt.y+1, pointsQueue, label);
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
