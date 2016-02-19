/*
Project : Image Stitching
Created By: Riley Fewell
Date 2/24/13
Cs 315 Spring 2013
*/



#include "EasyBMP.h"
#include "EasyBMP.cpp"
#include <iostream>
#include <cstdlib>
using namespace std;

// Filenames for input and output
const string INFILENAMES [16] = { "images/img1.bmp", "images/img2.bmp", "images/img3.bmp",
				  "images/img4.bmp", "images/img5.bmp", "images/img6.bmp", "images/img7.bmp", "images/img8.bmp",
				  "images/img9.bmp", "images/img10.bmp", "images/img11.bmp", "images/img12.bmp",
				  "images/img13.bmp", "images/img14.bmp", "images/img15.bmp", "images/img16.bmp" };

const string OUTFILENAME = "output.bmp";


// Compares the top edge of UseTop to the bottom edge of UseBottom.
// Assumes both inputs are squares of same size
// score obtained by adding the difference between color components
// squaring not used so that the numbers don't become too large
int rowMatch (BMP& UseTop, BMP& UseBottom)
{ 
  int height = UseTop.TellHeight();
  int width = UseBottom.TellWidth();
  int RowScore = 0;
  int Red = 0;
  int Green = 0;
  int Blue = 0;
  for (int i = 0; i < height-1; i++) {
    Red = abs(UseTop(i, 0)->Red - UseBottom(i, height-1)->Red);
    Green = abs(UseTop(i, 0)->Green - UseBottom(i, height-1)->Green);
    Blue = abs(UseTop(i, 0)->Blue - UseBottom(i, height-1)->Blue);
    RowScore += Red + Green + Blue;
  }
  return RowScore;
}


// Compares the top edge of UseTop to the bottom edge of UseBottom.
// Assumes both inputs are squares of same size
// score obtained by adding the difference between color components
// similar to the rowMatch
int columnMatch ( BMP& UseRight, BMP& UseLeft )
{
  
  int height = UseRight.TellHeight();
  int width = UseLeft.TellWidth();
  int ColScore = 0;
  int Red = 0;
  int Green = 0;
  int Blue = 0;
  for (int i = 0; i < width-1; i++) {
    Red = abs(UseRight(width-1, i)->Red - UseLeft(0, i)->Red);
    Green = abs(UseRight(width-1, i)->Green - UseLeft(0, i)->Green);
    Blue = abs(UseRight(width-1, i)->Blue - UseLeft(0, i)->Blue);
    ColScore += Red + Green + Blue;
  }
  return ColScore;
}

// Uses the rowMatch and columnMatch to create scores
// score[0][i][j] is the EAST SCORE and score[1][i][j] is the NORTH SCORE
// no need for WEST score and SOUTH score since you can get them from these
int finalScore (BMP images[16], int score[2][16][16]) {

  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      if (i == j)
	score[0][i][j] = 182596;  // just some large number
      else
	score[0][i][j] = columnMatch(images[i], images[j]);

  for (int i = 0; i < 16; i++)
    for (int j = 0; j < 16; j++)
      if (i == j)
	score[1][i][j] = 182596;  // just some large number
      else
	score[1][i][j] = rowMatch(images[i], images[j]);
}

// Finds the North West tile by adding the best North and West scores for each tile and
// choosing the one that maximizes the score
int findNorthWest(int score[2][16][16])
{
  int bestScoreIndex[16];
  int bestScore[16];
  for (int i = 0; i < 16; i++) {
    int sum = 100000000;
    for (int j = 0; j < 16; j++) {
      if ((score[0][i][j] + score[1][i][j]) < sum) {
	sum = score[0][i][j] + score[1][i][j];
	bestScoreIndex[i] = j;
	bestScore[i] = sum;
      }
    }
  }
    
  int highestScoreIndex = 0;
  int highestTempScore = 0;
    
  for (int k = 0; k < 16; k++) {
    if (bestScore[k] > highestTempScore) {
      highestScoreIndex = bestScoreIndex[k];
      highestTempScore = bestScore[k];
    }
  }
  return highestScoreIndex;
}

// for a given tile, find its eastern neighbor among the remaining ones
// remaining[j] is true for tiles that have not yet been placed in the final image
int findEastNeighbor(int score[2][16][16], int tile, bool remaining[16])
{   
  int loweast = 1000000000;                // random very large number
  int lowesteast = 0;
  for (int j = 0; j < 16; j++) {
    if (remaining[j]) {
      if (score[0][tile][j] < loweast) {
	loweast = score[0][tile][j];
	lowesteast = j;
      }
    }
  }
  return lowesteast;
}

// for a given tile, find its southern neighbor, among the remaining ones
// remaining[j] is true for tiles that have not yet been selected for placement
// similar to findEastNeighbor
int findSouthNeighbor(int score[2][16][16], int tile, bool remaining[16]) {

  int lowsouth = 100000000;               // random very large number
  int lowestsouth = 0;
  for (int j = 0; j < 16; j++) {
    if (remaining[j]) {
      if (score[1][j][tile] < lowsouth) {
	lowsouth = score[1][j][tile];
	lowestsouth = j;
      }
    }
  }
  return lowestsouth;
}


void copy(BMP & InImg, BMP & OutImg, int i, int j)
{
  int k = InImg.TellWidth();
  RangedPixelToPixelCopy(InImg, 0, k-1, k-1, 0, OutImg, j*k, i*k);
}


void greedy(int score[2][16][16], BMP InImg[16], BMP & OutImg) {
  bool remaining[16]; int temp[16];
  for (int j = 0; j < 16; j++)
    remaining[j] = true;
  int currentTile = findNorthWest(score);
  int k = 0;
  for (int p = 0; p < 4; p++)
    for (int q = 0; q < 4; q++)
      if (p == 0 && q == 0) {
	copy(InImg[currentTile], OutImg, 0, 0);     // fill the NW corner using InImg[j]
	remaining[currentTile] = false; temp[k++] = currentTile;
      } else if (q > 0) {                             // not done with the current row
	currentTile = findEastNeighbor(score, currentTile, remaining);
	copy(InImg[currentTile], OutImg, p, q); temp[k++] = currentTile;
	remaining[currentTile] = false;
      } else {                                        // start of a new row
	currentTile = findSouthNeighbor(score, temp[k-4], remaining);
	copy(InImg[currentTile], OutImg, p, q); temp[k++] = currentTile;
	remaining[currentTile] = false;
      }
} // end greedy

int main()
{
  BMP InImg[16], OutImg;                      // vector of input images and output image
  int score [2][16][16] ;                     // holds EAST and NORTH scores
  for( int i=0; i<16; ++i )                   // Read in the sub-images
    InImg[i].ReadFromFile(INFILENAMES[i].c_str());
  int subsize = InImg[0].TellWidth();
  OutImg.SetSize(4*subsize, 4*subsize);     // Set size of output image
  finalScore(InImg, score);
  greedy(score, InImg, OutImg);
  OutImg.WriteToFile(OUTFILENAME.c_str());
  return 0;
}
