#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef int bool;
#define true 1
#define false 0
#define MAX_REGIONS 1000
#define MAX_POINTS 50

typedef struct point1
{
	double x;
	double y;
}points;

/*container for regions*/
typedef struct regionData1
{
	double minRow;
	double maxRow;
	double minCol;
	double maxCol;
	int count;
	points **vertices;
	void *data;
	int region_id;		//indicates which region the node belongs to
}regionData;


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

double scalex(double x)
{
    double dimx = 13.361;
    double i = 4096;
    double coeff = i/dimx;
	double newx = (x*coeff);

	return newx;
}

double scaley(double y)
{
    double dimy = 25.399;
    double i = 4096;
    double coeff = i/dimy;
	double newy = (y*coeff);
	//printf("\nscaled y: %d", newy);

	return newy;
}

/*parse out keywords and return data*/
regionData *parseString(char *lineptr, char *keywordptr)
{
    int i, j;
    int count = 0;
    int linepos;
    int vertexcount=0;
    int found = 0;	//bool for whether substring is found in the string
    int pair = 0; //bool for first/second of pair - 0=first, 1=second
   

    char buffer[1024];
    double saveint[MAX_POINTS*2];

    regionData *regionptr = malloc(sizeof(regionData));		//pointer to a region
    points **vertexptr = malloc(sizeof(points*)*MAX_POINTS);	//pointer to pointer to array of all the points for a region
    points *rect;	//pointer to structure

    /*assign local pointers*/
    char *line = lineptr;
    char *word = keywordptr;

    //printf("String: %s\n",line);
  	//printf("Substring: %s\n",word);
    
    /*check if the string (line) is longer than the substring (word) - 
    if so, word is a valid substring (else return error)*/
    if (strlen(line) >= strlen(word)) 
    {
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
    {
         printf("keyword not found\n");
    }


    if(found==1)
    {
        for(i=linepos, j=0; i<strlen(line); i++)
        {
            /*if the character matches int 0-9, increase counter position*/
            if((line[i] >= '0' && line[i] <= '9' )|| line[i] == '.')
            {
                buffer[j] = line[i];
               // printf("\nbuffer[%d]: %d", j, buffer[j]);
                j++;
            }
            else
            {
                if(j>0)
                {
                	/*convert found ints, reset integer counter*/
                    buffer[j] = '\0';
                    saveint[count] = atof(buffer);
                    //printf("\nsaveint[%d]: %.4f", count, saveint[count]);
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
    	   	 	pair = 1;
        	 }
        	 else
        	 {
        	 	rect->y = saveint[i];
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

        // for(i=0; i<vertexcount; i++)
        // {
        //     if(regionptr->vertices[i]->x < min_x)
        //         min_x = regionptr->vertices[i]->x;
        //     else if(regionptr->vertices[i]->x > max_x)
        //         max_x = regionptr->vertices[i]->x;

        //     if(regionptr->vertices[i]->y < min_y)
        //         min_y = regionptr->vertices[i]->y;
        //     else if(regionptr->vertices[i]->y > max_y)
        //         max_y = regionptr->vertices[i]->y;
        // }

        // regionptr->minRow = min_x;
        // regionptr->maxRow = max_x;
        // regionptr->minCol = min_y;
        // regionptr->maxCol = max_y;

        return regionptr;
    }
}

int main(int argc, char *argv[])
{
	int i;
	char buffer[4096];
	char keyword1[128] = "POLYGON";			//set keyword to look for
	int count = 0;
	int verts;
	int newx, newy;
    double dimx, dimy;
	
    int prevx, prevy;

    double min_x = 10000000;
    double max_x = 0;
    double min_y = 10000000;
    double max_y = 0;

    double newmin_x = 0;
    double newmax_x;
    double newmin_y=0;
    double newmax_y;

	regionData **rect = malloc(sizeof(regionData*)*MAX_REGIONS);

	FILE *infile;
	infile = fopen(argv[1], "r");

	FILE *outfile;
	outfile = fopen("buildings.wkt", "w");

	if(infile == NULL)
	{
		perror("Error opening file");
		return 0;
	}	
	else
	{
		while(fgets(buffer, sizeof(buffer), infile))
		{
			if (strlen(buffer) >= strlen(keyword1))
			{
				//printf("buffer size: %lu", strlen(buffer));
          
				rect[count] = parseString(buffer, keyword1);
				
                for(int i=0; i<rect[count]->count; i++)
                {
                    if(rect[count]->vertices[i]->x < min_x)
                        min_x = rect[count]->vertices[i]->x;
                    
                    if(rect[count]->vertices[i]->x > max_x)
                        max_x = rect[count]->vertices[i]->x;

                    if(rect[count]->vertices[i]->y < min_y)
                        min_y = rect[count]->vertices[i]->y;
                    
                    if(rect[count]->vertices[i]->y > max_y)
                        max_y = rect[count]->vertices[i]->y;
                }
        
            verts = 0;
            fprintf(outfile, "POLYGON ((");
            for(i=0; i<rect[count]->count; i++)
            { 
                dimx = (rect[count]->vertices[i]->x)-min_x;
                dimy = (rect[count]->vertices[i]->y)-min_y;
                newx = (int)scalex(dimx);
                newy = (int)scalex(dimy);

            //newx = scalex(rect[count]->vertices[i]->x);
            //newy = scaley(rect[count]->vertices[i]->y);
            //fprintf(outfile,"[x y]: [%d %d]\n", newx, newy);      

                //if(newx != prevx && newy != prevy)
                {
                    if(i>0)
                        fprintf(outfile, ", ");
                    
                    fprintf(outfile, "%d %d", newx, newy);  
                    
                    //prevx = newx;
                    //prevy = newy;
                    verts++;
                }   
            }

           // if(verts==1)
           //  {
           //     fprintf(outfile, "%f %f", newx, newy);
           //  }

            fprintf(outfile, "))\n\n");
            count++;

            if(count == MAX_REGIONS)
                break; 
            }
        }

        for(i=count; i<MAX_REGIONS; i++)
        {
            rect[count] = NULL;
        }

        printf("min x: %f", min_x);
        printf("\nmax x: %f", max_x);
        printf("\nmin y: %f", min_y);
        printf("\nmax y: %f\n", max_y);

        newmax_x = max_x-min_x;
        newmax_y = max_y-min_y;

        printf("\nnew min x: %f", newmin_x);
        printf("\nnew max x: %f", newmax_x);

        printf("\nnew min y: %f", newmin_y);
        printf("\nnew max y: %f", newmax_y);	
	
	   fclose(infile);
	   fclose(outfile);
	   return 1;
    }   
}