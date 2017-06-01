#ifndef _QUADTREE_H_INCLUDED_
#define _QUADTREE_H_INCLUDED_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <GL/glut.h>

typedef int bool;
#define true 1
#define false 0
#define MAX_REGIONS 500
#define MAX_POINTS 20
#define INF 10000


int row, col;
int ptx, pty;
int x1, y1;
int nodecount, nodecount1;

int region_count;
int region_x[MAX_REGIONS][MAX_POINTS];
int region_y[MAX_REGIONS][MAX_POINTS];

typedef struct point1 points;
typedef struct regionData1 regionData;
typedef struct boundingBox1 boundingBox;
typedef struct quadTreeNode1 quadTreeNode;


//graphics globals
// int winw, winh; 
// int mousex, mousey;
FILE *txtfile;
quadTreeNode *root;

boundingBox createBoundingBox(int begx, int begy, int endx, int endy);
quadTreeNode *createQuadTreeNode(boundingBox boundary);

int max(int a, int b);
int min(int a, int b);
int StringtoInt (char *stringptr);

bool onSegment(points p, points q, points r);
int orientation(points p, points q, points r);
bool doIntersect(points p1, points q1, points p2, points q2);
bool isInside(points *polygon, int n, points p);
int divide(quadTreeNode *node, regionData *region);

int findNode(int x1, int y1, quadTreeNode *topnode);
void sizeOf(quadTreeNode *topnode);
//void presizeOf(quadTreeNode *topnode);
//translate function
int merge(quadTreeNode *topnode);
int traverseTree(quadTreeNode *topnode);

regionData *parseString(char *lineptr, char *keywordptr);
regionData **readfile(FILE *infile, char *keyword);

void translateTree(quadTreeNode *topnode);
void translateRegion(quadTreeNode *topnode, int *region_found);

//Graphics call
int displayTree(quadTreeNode *topnode);
void init2D(float r, float g, float b);
void display(void);

quadTreeNode *mapRegions(char *filename1, char *filename2, int endx, int endy);

#endif /*_QUADTREE_H_INCLUDED*/