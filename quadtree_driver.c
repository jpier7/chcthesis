#include "quadtree.h"

int main(int argc, char *argv[])
{
	quadTreeNode *node1;

	//FILE *infile = fopen(argv[1], "r");
	int row=atoi(argv[3]);
	int col=atoi(argv[4]);
	node1 = mapRegions(argv[1], argv[2], row-1,col-1);
	//TODO: FREE MEMORY!
	
	free(node1);

}