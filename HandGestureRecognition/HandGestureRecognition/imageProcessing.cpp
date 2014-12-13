#include "stdafx.h"
#include "imageProcessing.h"
#include <queue>

using namespace std;

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
        //aux[i][j] = true;
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
void checkNeighbour(bool** binaryImage, int** labelImage, int xCoord, int yCoord, queue<Point2D>& pointsQueue, int label) {
  if ( binaryImage[yCoord][xCoord] && labelImage[yCoord][xCoord] == 0 ) {
    labelImage[yCoord][xCoord] = label;
    Point2D newPt; newPt.x = xCoord; newPt.y = yCoord;
    pointsQueue.push(newPt);
  }
}

void labelEntireObject(bool** binaryImage, int** labelImage, Point2D startPoint, int label) {
  queue<Point2D> pointsQueue;
  pointsQueue.push(startPoint);
  while( !pointsQueue.empty() ) {
    Point2D pt = pointsQueue.front();
    pointsQueue.pop();

    checkNeighbour(binaryImage, labelImage, pt.x-1, pt.y, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x+1, pt.y, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x, pt.y+1, pointsQueue, label);
    checkNeighbour(binaryImage, labelImage, pt.x, pt.y-1, pointsQueue, label);
  }
}

//scan the image until you find a 
void labelImage(bool** binaryImage, int** labelImage, int height, int width) {
  initializeLabelImage(labelImage, height, width);
  
  int label = 0;

  for (int i = 1; i < height-1; i++) {
    for (int j = 1; j < width-1; j++) {
      if (binaryImage[i][j] && (labelImage[i][j] == 0)) {//if not labeled foreground pixel
        label++;
        Point2D startPoint; startPoint.x = j; startPoint.y = i;
        labelImage[startPoint.y][startPoint.x] = label;
        labelEntireObject(binaryImage, labelImage, startPoint, label);
      } 
    }
  }
}

