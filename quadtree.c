#include "quadtree.h"

struct point1
{
	int x;
	int y;
};

/*container for regions*/
struct regionData1
{
	int minRow;
	int maxRow;
	int minCol;
	int maxCol;
	int count;
	points **vertices;
	void *data;
	int region_id;		//indicates which region the node belongs to
};

/*container for quadrant boundaries*/
struct boundingBox1		
{
	int begx;
	int begy; 
	int endx;
	int endy;
};

/*container for region boundaries*/
struct quadTreeNode1
{
	struct quadTreeNode1 *nw;
	struct quadTreeNode1 *ne;
	struct quadTreeNode1 *sw;
	struct quadTreeNode1 *se;
	boundingBox boundary;
	regionData *points;
};

/*create a boundary*/
boundingBox createBoundingBox(int begx, int begy, int endx, int endy)
{
	boundingBox bb;
	bb.begx = begx;
	bb.begy = begy;
	bb.endx = endx;
	bb.endy = endy;
	return bb;
}

/*create a node in the tree*/
quadTreeNode *createQuadTreeNode(boundingBox boundary)
{
	quadTreeNode *node = malloc(sizeof(quadTreeNode));
	node->nw = NULL;
	node->ne = NULL;
	node->sw = NULL;
	node->se = NULL;

	node->boundary = boundary;
	node->points = NULL;
	return node;
}

int max(int a, int b)
{
   return (a>=b)? a:b;
}

int min(int a, int b)
{
   return (a<=b)? a:b;
}

/*take in a string, convert to int*/
int StringtoInt(char *stringptr)
{
	int n, i;

	for(i=0, n=0; i<strlen(stringptr); i++)
	{
		n=(n*10)+(stringptr[i]-48);
	}
	return n;
}

int find_power(int dimx, int dimy)
{
	/*TODO: 
	given dimx and dimy:
	1. dimx != dimy
	2. figure out which is bigger, dimx or dimy
	3. calculate larger dimension to nearest power of 2
	4. set both dimx and dimy to be the nearest power of 2*/

	float i;

	//1st case: x dimension larger than y dimension OR x dim == y dim
	if(dimx > dimy || dimx == dimy)
	{	
		i = (ceil(log(dimx)/log(2)));
		dimx = (int)(pow(2, i));
		return dimx;
	}

	//2nd case: y dimension larger than x dimension
	if(dimy > dimx)
	{
		i = (ceil(log(dimy)/log(2)));
		dimy = (int)(pow(2, i));
		return dimy;
	}
}

// Given three colinear points p, q, r, the function checks if
// point q lies on line segment 'pr'
bool onSegment(points p, points q, points r)
{
   if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y))
      return true;
   return false;
}

// To find orientation of ordered triplet (p, q, r).
// The function returns following values
// 0 --> p, q and r are colinear
// 1 --> Clockwise
// 2 --> Counterclockwise
int orientation(points p, points q, points r)
{
// See http://www.geeksforgeeks.org/orientation-3-ordered-points/
// for details of below formula.
   int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);
   if (val == 0) return 0;  // colinear
   return (val > 0)? 1: 2; // clock or counterclock wise
}

// The main function that returns true if line segment 'p1q1'
// and 'p2q2' intersect.
bool doIntersect(points p1, points q1, points p2, points q2)
{
// Find the four orientations needed for general and
// special cases
   int o1 = orientation(p1, q1, p2);
   int o2 = orientation(p1, q1, q2);
   int o3 = orientation(p2, q2, p1);
   int o4 = orientation(p2, q2, q1);
// General case
   if (o1 != o2 && o3 != o4)
      return true;
// Special Cases
// p1, q1 and p2 are colinear and p2 lies on segment p1q1
   if (o1 == 0 && onSegment(p1, p2, q1)) return true;
// p1, q1 and p2 are colinear and q2 lies on segment p1q1
   if (o2 == 0 && onSegment(p1, q2, q1)) return true;
// p2, q2 and p1 are colinear and p1 lies on segment p2q2
   if (o3 == 0 && onSegment(p2, p1, q2)) return true;
// p2, q2 and q1 are colinear and q1 lies on segment p2q2
   if (o4 == 0 && onSegment(p2, q1, q2)) return true;
   return false; // Doesn't fall in any of the above cases
}

bool isInside(points *polygon, int n, points p)
{
   if (n<3) return false;
   points extreme = {INF, p.y};
   int count = 0, i=0;
   do
   {
      int next = (i+1)%n;
      if (doIntersect(polygon[i], polygon[next], p, extreme))
      {
         if (orientation(polygon[i],p,polygon[next]) == 0)
            return onSegment(polygon[i],p,polygon[next]);
         count ++;
      }
      i = next;
   } while (i != 0);
   return count&1;
}

/*recursively divide into 4 quadrants*/
/*return: 
	0 if region NOT in subquadrant
	1 if region IS in subquadrant
	10 if max region IS in subquadrant but actual region is NOT in subquadrant*/
int divide(quadTreeNode *node, regionData *region)
{
	int return_code = 0;
	/*set variables for node:
	begx = first row; begy = first col; endx =last row, endy = last col*/
	int begx = node->boundary.begx;
	int begy = node->boundary.begy;
	int endx = node->boundary.endx;
	int endy = node->boundary.endy;
	points begpt, endpt;

	/*calculate midpoint of quadrant*/
	int midx = ((endx-begx)+1)/2;		
	int midy = ((endy-begy)+1)/2;

	/*if the midpoint of the x (rows) or the midpoint of the y (columns) is 0, done dividing*/
	if(midx==0 || midy==0)
	{
		/*check that the leaf is contained in the polygon*/
		//TODO: make loop for polygon array - initialize all to NULL or 0
		points polygon[MAX_POINTS];
		begpt.x=(begx+endx)/2;
		begpt.y=(begy+endy)/2;
		for(int i=0; i<region->count; i++)
		{
			polygon[i].x=region->vertices[i]->x;
			polygon[i].y=region->vertices[i]->y;
		}

		if(isInside(polygon, region->count,begpt))
		{
			node->points = region;
			return 1;
		}
		else
		{
			return 10;
		}
	}
	
	/*printf("\nnode boundary: [%d,%d],[%d,%d]\n",begx,begy,endx,endy);
	printf("nw boundary: [%d,%d],[%d,%d]\n",begx,begy,((begx+midx)-1),((begy+midy)-1));
	printf("ne boundary: [%d,%d],[%d,%d]\n",begx,begy+midy,((begx+midx)-1),endy);
	printf("sw boundary: [%d,%d],[%d,%d]\n", begx+midx,begy, endx,((begy+midy)-1));
	printf("se boundary: [%d,%d],[%d,%d]\n", begx+midx,begy+midy, endx, endy);

/*create quad tree with regions as nodes*/
	
	/*NORTHWEST QUADRANT*/
	if(begx<=region->maxRow && ((begx+midx)-1)>=region->minRow && begy<=region->maxCol && ((begy+midy)-1)>=region->minCol)
	{
		//printf("\nsub nw:");
		if(node->nw == NULL)
		{
			/*create the boundary*/
			boundingBox boundNW = createBoundingBox(begx, begy, (begx+midx)-1, (begy+midy)-1);		
			/*create the node -- createQuadTreeNode takes boundary as parameter*/
			quadTreeNode *nodeNW = createQuadTreeNode(boundNW);
			node->nw = nodeNW;			
		}
		return_code=divide(node->nw, region);

		if(return_code==10)
			node->nw = NULL;
	}
	
	/*NORTHEAST QUADRANT*/
	if(begx<=region->maxRow && ((begx+midx)-1)>=region->minRow && (begy+midy)<=region->maxCol && endy>=region->minCol)	
	{
		//printf("\nsub ne:");
		if(node->ne == NULL)
		{
			/*create the boundary*/
			boundingBox boundNE = createBoundingBox(begx, begy+midy, (begx+midx)-1, endy);		
			/*create the node -- createQuadTreeNode takes boundary as parameter*/
			quadTreeNode *nodeNE = createQuadTreeNode(boundNE);	
			node->ne = nodeNE;

		}
		return_code=divide(node->ne, region);

		if(return_code==10)
			node->ne = NULL;
	}
	
	/*SOUTHWEST QUADRANT*/
	if((begx+midx)<=region->maxRow && endx>=region->minRow && begy<=region->maxCol && (begy+midy)-1>=region->minCol)	
	{
		//printf("\nsub sw:");
		if(node->sw == NULL)
		{
			/*create the boundary*/
			boundingBox boundSW = createBoundingBox(begx+midx, begy, endx, (begy+midy)-1);		
			/*create the node -- createQuadTreeNode takes boundary as parameter*/
			quadTreeNode *nodeSW = createQuadTreeNode(boundSW);	
			node->sw = nodeSW;
		}
		return_code=divide(node->sw, region);

		if(return_code==10)
			node->sw = NULL;
	}

	/*SOUTHEAST QUADRANT*/
	if((begx+midx)<=region->maxRow && endx>=region->minRow && begy+midy<=region->maxCol && endy>=region->minCol)
	{
		//printf("\nsub se:");
		if(node->se == NULL)
		{
			/*create the boundary*/
			boundingBox boundSE = createBoundingBox(begx+midx, begy+midy, endx, endy);		
			/*create the node -- createQuadTreeNode takes boundary as parameter*/
			quadTreeNode *nodeSE = createQuadTreeNode(boundSE);
			node->se = nodeSE;
		}
		return_code=divide(node->se, region);
		if(return_code==10)
			node->se = NULL;
	}

	return 0;
}

/*return the desired node*/
int findNode(int x1, int y1, quadTreeNode *topnode)
{
	int found=0;
	int begx = topnode->boundary.begx;
	int begy = topnode->boundary.begy;
	int endx = topnode->boundary.endx;
	int endy = topnode->boundary.endy;

	/*calculate midpoint of quadrant*/
	int midx = ((endx-begx)+1)/2;		
	int midy = ((endy-begy)+1)/2;

	//NORTHWEST QUADRANT SEARCH
	if(x1>=begx && x1 <=(begx+midx)-1 && y1>=begy && y1<=(begy+midy)-1)
	{
		if(topnode->nw !=NULL)
			findNode(x1, y1, topnode->nw);
		
		//found a match
		else if(topnode->nw == NULL && x1>=begx && x1 <=endx && y1>=begy && y1<=endy)
		{
			found=1;
			printf("\npoint [%d %d] exists within node [%d %d][%d %d]", x1, y1, begx, begy, endx, endy);
		}
		
		//else this point is not part of a region that exists within the tree.
		else
		{
			found=0;
			printf("\npoint [%d %d] not in boundary.", x1, y1);
		}
	}

	//NORTHEAST QUADRANT SEARCH
	if(x1>=begx && x1<=(begx+midx)-1 && y1>=(begy+midy) && y1<=endy)
	{
		if(topnode->ne != NULL)
			findNode(x1, y1, topnode->ne);
		else if(topnode->ne == NULL && x1>=begx && x1 <=endx && y1>=begy && y1<=endy)
		{
			found=1;
			printf("\npoint [%d %d] exists within node [%d %d][%d %d]", x1, y1, begx, begy, endx, endy);
		}
		else
		{
			found=0;
			printf("\npoint [%d %d] not in boundary.", x1, y1);
		}
	}

	//SOUTHWEST QUADRANT SEARCH
	if(x1>=(begx+midx) && x1<=endx && y1>=begy && y1<=(begy+midy)-1)
	{
		if(topnode->sw != NULL)
			findNode(x1, y1, topnode->sw);
		else if(topnode->nw == NULL && x1>=begx && x1 <=endx && y1>=begy && y1<=endy)
		{
			found=1;
			printf("\npoint [%d %d] exists within node [%d %d][%d %d]", x1, y1, begx, begy, endx, endy);
		}
		else
		{
			found=0;
			printf("\npoint [%d %d] not in boundary.", x1, y1);
		}
	}

	//SOUTHEAST QUADRANT SEARCH
	if(x1>=(begx+midx) && x1<=endx && y1>=(begy+midy) && y1<=endy)
	{
		if(topnode->se != NULL)
			findNode(x1, y1, topnode->se);
		else if(topnode->nw == NULL && x1>=begx && x1 <=endx && y1>=begy && y1<=endy)
		{
			found=1;
			printf("\npoint [%d %d] exists within node [%d %d][%d %d]", x1, y1, begx, begy, endx, endy);
		}
		else
		{
			found=0;
			printf("\npoint [%d %d] not in boundary.", x1, y1);
		}
	}

	return 1;
}

void sizeOf(quadTreeNode *topnode)
{
	//count the number of nodes by postorder
	if(topnode == NULL)
		return;

	sizeOf(topnode->nw);
	sizeOf(topnode->ne);
	sizeOf(topnode->sw);
	sizeOf(topnode->se);
	nodecount++;
	return;
}

/*void presizeOf(quadTreeNode *topnode)
{
	if(topnode == NULL)
		return;

	nodecount1++;
	presizeOf(topnode->nw);
	presizeOf(topnode->ne);
	presizeOf(topnode->sw);
	presizeOf(topnode->se);
	return;
}*/

/*void translate(//??)
{
		/*TODO:
        	for vertex count: 
        	create temp array

	REVERSE:
        	Copying elements into array b starting from end of array  
   				for (c = n - 1, d = 0; c >= 0; c--, d++)
      				b[d] = a[c];
 
 			Copying reversed array into original.
    		Here we are modifying original array, this is optional.
   				for (c = 0; c < n; c++)
      			a[c] = b[c];

    TRANSLATE COORDINATES:
    switch x and y (x, y) -> (y, x)
 			
}*/

int merge (quadTreeNode *topnode)
{
int leaf_found;
int leaf_count;

	leaf_count = 0;
	if(topnode->nw != NULL) //pointer to more nodes
	{
//printf("before nw: %d\n",topnode->nw);
		leaf_found=merge(topnode->nw);
		leaf_count=leaf_count+leaf_found;
//printf("after nw: %d\n",topnode->nw);
//printf("leaf_found: %d\n",leaf_found);
//printf("leaf_count: %d\n",leaf_count);

	}
	if(topnode->ne != NULL)
	{
//printf("before ne: %d\n",topnode->ne);
		leaf_found=merge(topnode->ne);
		leaf_count=leaf_count+leaf_found;
//printf("after ne: %d\n",topnode->ne);
//printf("leaf_found: %d\n",leaf_found);
//printf("leaf_count: %d\n",leaf_count);
	}
	if(topnode->sw != NULL)
	{
//printf("before sw: %d\n",topnode->sw);
		leaf_found=merge(topnode->sw);
		leaf_count=leaf_count+leaf_found;
//printf("after sw: %d\n",topnode->sw);
//printf("leaf_found: %d\n",leaf_found);
//printf("leaf_count: %d\n",leaf_count);

	}
	if(topnode->se !=NULL)
	{
//printf("before se: %d\n",topnode->se);
		leaf_found=merge(topnode->se);
		leaf_count=leaf_count+leaf_found;
//printf("after se: %d\n",topnode->se);
//printf("leaf_found: %d\n",leaf_found);
//printf("leaf_count: %d\n",leaf_count);

	}

	//if all the pointers are null, we have found a leaf.
	if(topnode->nw == NULL && topnode->ne == NULL && topnode->sw == NULL && topnode->se == NULL)
	{
		//printf("\ntopnode: %d",topnode);
		//printf("\ntopnode->points: %d", topnode->points);
		//printf("\ntop boundary: [%d, %d]", topnode->boundary.begx, topnode->boundary.begy);
		//printf("\nbottom boundary: [%d, %d]\n", topnode->boundary.endx, topnode->boundary.endy);
		return 1;
	}

	if (leaf_count == 4)
	{
		if((topnode->ne != NULL)  && (topnode->ne->points == topnode->nw->points))
		{
			if((topnode->sw != NULL) && (topnode->sw->points == topnode->nw->points))
			{
				if((topnode->se != NULL) && (topnode->se->points == topnode->nw->points))
				{
//printf("4 leaves merged: %d, %d, %d, %d\n",topnode->nw, topnode->ne, topnode->sw, topnode->se);
//printf("region is: %d\n",topnode->nw->points);
					topnode->points = topnode->nw->points;
					topnode->nw = NULL;
					topnode->ne = NULL;
					topnode->sw = NULL;
                    topnode->se = NULL;
                    //printf("\ntop boundary: [%d, %d]", topnode->boundary.begx, topnode->boundary.begy);
					//printf("\nbottom boundary: [%d, %d]\n", topnode->boundary.endx, topnode->boundary.endy);
					return 1;
				}
			}
		}
	}	

	return 0;
}

int traverseTree(quadTreeNode *topnode)
{	

	if(topnode->nw != NULL) //pointer to more nodes
	{
//printf("before nw: %d\n",topnode->nw);
		traverseTree(topnode->nw);
//printf("after nw: %d\n",topnode->nw);
	}
	if(topnode->ne != NULL)
	{
//printf("before ne: %d\n",topnode->ne);
		traverseTree(topnode->ne);
//printf("after ne: %d\n",topnode->ne);
	}
	if(topnode->sw != NULL)
	{
//printf("before sw: %d\n",topnode->sw);
		traverseTree(topnode->sw);
//printf("after sw: %d\n",topnode->sw);
	}
	if(topnode->se !=NULL)
	{
//printf("before se: %d\n",topnode->se);
		traverseTree(topnode->se);
//printf("after se: %d\n",topnode->se);
	}

	//if all the pointers are null, we have found a leaf.
	if(topnode->nw == NULL && topnode->ne == NULL && topnode->sw == NULL && topnode->se == NULL)
	{
		//printf("\ntopnode: %d",topnode);
		//printf("\ntopnode->points: %d", topnode->points);
		//printf("\ntop boundary: [%d, %d]", topnode->boundary.begx, topnode->boundary.begy);
		//printf("\nbottom boundary: [%d, %d]\n", topnode->boundary.endx, topnode->boundary.endy);
		//FREE?
	}
	return 1;
}


/*parse out keywords and return data*/
regionData *parseString(char *lineptr, char *keywordptr)
{
    int i, j;
    int min_x = 1000;
    int min_y = 1000;
    int max_x = 0;
    int max_y = 0;
    int count = 0;
    int linepos;
    int vertexcount=0;
    int found = 0;	//bool for whether substring is found in the string
    int pair = 0; //bool for first/second of pair - 0=first, 1=second

    char buffer[1024];
    int saveint[MAX_POINTS*2];
    regionData *regionptr;
    points **vertexptr;
    points *rect;	//pointer to structure

    /*assign local pointers*/
    char *line = lineptr;
    char *word = keywordptr;

    //printf("String: %s\n",line);
  	//printf("Substring: %s\n",word);
    
    /*check if the string (line) is longer than the substring (word) - 
    if so, word is a valid substring (else return error)*/
  // printf("string length: %lu\nword length: %lu\n", strlen(line), strlen(word));
    //printf("string: %s\nword:%s\n", line, word);

    if (strlen(line) >= strlen(word)) 
    {
    	regionptr = malloc(sizeof(regionData));		//pointer to a region
    	vertexptr = malloc(sizeof(points*)*MAX_POINTS);	//pointer to pointer to array of all the points for a region
        for (i=0, j=0; i<strlen(line); i++) 
        {
        	/*if characters match, increase word position*/
             if (line[i] = word[j]) 
             {
                j++;

            /*if we reached the end of the word, mark word as found*/
                if(strlen(word)==j)
                {
                    found=1;
                    linepos=i;
                    break;
                }
             }
             else 
             {
                j=0;
             }
        }
    } 
    else
    	return NULL;
         //printf("keyword not found\n");


    if(found==1)
    {
        for(i=linepos, j=0; i<strlen(line); i++)
        {
            /*if the character matches int 0-9, increase counter position*/
            if(line[i] >= '0' && line[i] <= '9')
            {
                buffer[j] = line[i];
                j++;
            }
            else
            {
                if(j>0)
                {
                	/*convert found ints, reset integer counter*/
                    buffer[j] = '\0';
                    saveint[count] = StringtoInt(buffer);
                    j=0;
                    count++;
                }

                /*delimiter for the end of the function*/
                if(line[i] == ')')
                {
                    break;
                }
            }
        }

        for(i=0; i<count; i++)
        {
        	 if(pair==0)
        	 {
        	 	/*if we find a number, */
        	 	rect = malloc(sizeof(points));
        	 	rect->x = saveint[i];
        	 	//printf("\nint x: %d", rect->x);
    	   	 	pair = 1;
        	 }
        	 else
        	 {
        	 	rect->y = saveint[i];
        	 	//printf("\nint y: %d", rect->y);
        	 	pair = 0;
        	 	vertexptr[vertexcount] = rect;
        	 	vertexcount++;
        	 }
        }

       //CHANGE: if too many points, error; invalid region if 1st and last points don't match

        /*for(j=vertexcount; j<MAX_POINTS; j++)
        {
        	//if(vertexptr[vertexcount] != vertexptr[j])
        		//printf("not a valid region");
      
       		vertexptr[vertexcount] = NULL;
        	printf("\nnot enough points\n");
        	

        }*/

        
        regionptr->vertices = vertexptr;
        regionptr->count = vertexcount;

        for(i=0; i<vertexcount; i++)
        {
        	if(regionptr->vertices[i]->x < min_x)
        		min_x = regionptr->vertices[i]->x;
        	else if(regionptr->vertices[i]->x > max_x)
        		max_x = regionptr->vertices[i]->x;

        	if(regionptr->vertices[i]->y < min_y)
        		min_y = regionptr->vertices[i]->y;
        	else if(regionptr->vertices[i]->y > max_y)
        		max_y = regionptr->vertices[i]->y;
        }

        regionptr->minRow = min_x;
        regionptr->maxRow = max_x;
        regionptr->minCol = min_y;
        regionptr->maxCol = max_y;

        return regionptr;
    }
    //return regionptr;
}

/* open file and read coordinates of rectangular regions*/
regionData **readfile(FILE *infile, char *keyword1)
{
	//int upleftx, uplefty, botrightx, botrighty;
	char buffer[1024];			//set keyword to look for
	int count = 0;
	regionData *temp;
	regionData **rect = malloc(sizeof(regionData*)*MAX_REGIONS);

	// FILE *file;
	// file = fopen(infile, "r");

	if(infile == NULL)
	{
		perror("Error opening file");
		return NULL;
	}	
	else
	{
		while(fgets(buffer, sizeof(buffer), infile))
		{
			//printf("buffer size: %lu", strlen(buffer));
			temp = parseString(buffer, keyword1);

			if (temp !=NULL)
			{
				rect[count] = parseString(buffer, keyword1);
				count++;	
				//printf("\ncount: %d", count);
			}

			if(count == MAX_REGIONS)
				break;

			//printf("\ncount: %d", count);

			if(count == MAX_REGIONS)
				break;
		}

		//printf("\ncount: %d", count);
		for(int i=count; i<MAX_REGIONS; i++)
		{
			rect[count] = NULL;
		}
	}	
	
	
	return rect;
}

//GRAPHICS CALLS:
/*void init2D(float r, float g, float b)
{
	glClearColor(r,g,b,0.0);  
	glMatrixMode (GL_PROJECTION);
	gluOrtho2D (0.0, 200.0, 0.0, 150.0);
}

// mouse
// The GLUT mouse function
void mouse(int x, int y)
{

   // Save the mouse position
   mousex = x;
   mousey = y;

   //create a mouse track file
   fprintf(file, "\nmouse: %d, %d", mousex, mousey);

   //show mouse position in stdout
   fprintf(stdout, "\nmouse: %d, %d", mousex, mousey);

}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 0.0, 0.0);

	// Convert mouse position to OpenGL's coordinate system
   	double oglx = (double)mousex/winw;
    double ogly = 1-(double)mousey/winh;
	glFlush();
}*/

void translateRegion(quadTreeNode *topnode, int *region_found)
{
int which_region;
int i;
int last_vertice;
	if(topnode->nw != NULL) //pointer to more nodes
		translateRegion(topnode->nw,region_found);
	
	if(topnode->ne != NULL)
		translateRegion(topnode->ne,region_found);

	if(topnode->sw != NULL)
		translateRegion(topnode->sw,region_found);

	if(topnode->se !=NULL)
		translateRegion(topnode->se,region_found);

	//if all the pointers are null, we have found a leaf.
	if(topnode->nw == NULL && topnode->ne == NULL && topnode->sw == NULL && topnode->se == NULL)
	{ 
//printf("in translateRegion-pointer to regionData: %d\n",topnode->points);
//printf("in translateRegion-pointer to region_id: %d\n",topnode->points->region_id);
/*
printf("region %d: %d\n",topnode->points->region_id, region_found[topnode->points->region_id]);
		which_region= topnode->points->region_id;
		if (region_found[which_region]==0)
		{
			region_found[which_region]=1;
			last_vertice=topnode->points->count;
			for (i=0; i<last_vertice; i++)
			{
				region_x[which_region][i]=topnode->points->vertices[last_vertice-i]->y;
				region_y[which_region][i]=topnode->points->vertices[last_vertice-i]->x;
			}
		}
for (i=0; i<MAX_REGIONS; i++)
{
	printf("region %d: %d",i,region_found[i]);
}
*/
		return;
	}
}

void translateTree(quadTreeNode *topnode)
{
	int i, j;
	int region_found[MAX_REGIONS];
	
//initialize array that flags which region has been translated for the graphics
	for (i=0; i<MAX_REGIONS; i++)
	{
		region_found[i]=0;
	}

//initialize array that holds vertices for each region 
	for (i=0; i<region_count; i++)
	{
		for (j=0; j<MAX_POINTS; j++)
		{
			region_x[i][j]=-1;
			region_y[i][j]=-1;
		}
	}
	

	for (i=0; i<region_count; i++)
	{
		translateRegion(topnode, region_found);
	}
	return;
}

quadTreeNode *mapRegions(char *filename1, char *filename2, int endx, int endy)
{

	int i;
	int divide_ret;
	nodecount = 0;
	//nodecount1 = 0; 
	clock_t t0, t1;
	double elapsed_time;
	char keyword[128] = "POLYGON";
	char keyword2[128] = "mouse";

	/*create the boundary*/
	boundingBox box = createBoundingBox(0, 0, endx, endy);	

	/*create the node -- createQuadTreeNode takes boundary as parameter*/
	quadTreeNode *node1 = createQuadTreeNode(box);	
	//root = node1;

	/*create the node data*/	
	FILE *infile1 = fopen(filename1, "r");
	FILE *infile2 = fopen(filename2, "r");

	regionData **datafile = readfile(infile1, keyword);
	regionData **datafile2 = readfile(infile2, keyword2);
	
	// fclose(infile1);
	// return node1;
	for(i=0; i<MAX_REGIONS; i++)
	{
		if(datafile[i] == NULL)
		{
	
			break;
		}
			
		else
		{
			datafile[i]->region_id=i;
			divide_ret=divide(node1, datafile[i]);
			//printf("\npreorder size: %d", nodecount1);

		}
	}
	region_count=i;
//printf("\nregion count: %d\n",region_count);

	//traverseTree(node1);
	merge(node1);
	traverseTree(node1);
	sizeOf(node1);
	//presizeOf(node1);

	t0 = clock();
	for(int j=0; j<MAX_REGIONS; j++)
	{
		if(datafile2[j] == NULL)
		{
			break;
		}

		else
		{
			findNode(datafile2[j]->vertices[0]->x, datafile2[j]->vertices[0]->y, node1);
			t1 = clock();
			elapsed_time = ((double)(t1-t0) / CLOCKS_PER_SEC);
			printf("\noperation time: %f seconds", elapsed_time);
			//t = clock();		
		}
	}
	
	
	printf("\nimage dimensions: %dx%d", endx+1, endy+1);
	printf("\nsize: %d nodes\n", nodecount);

	fclose(infile1);
	fclose(infile2);
	return node1;
}