/*MOUSE TRACKING
> continuously updates cursor position on screen
> prints current mouse coordinates to stdout
> saves "mouse track" into a file*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glut.h>

int winw, winh; 
int mousex, mousey;
FILE *file;

void init2D(float r, float g, float b)
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
   fprintf(file, "\nmouse: %d %d", mousex, mousey);
   fprintf(stdout, "\nmouse: %d %d", mousex, mousey);

}


void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glColor3f(1.0, 0.0, 0.0);

	// Convert mouse position to OpenGL's coordinate system
   	double oglx = (double)mousex/winw;
    double ogly = 1-(double)mousey/winh;
	glFlush();
}


void main(int argc,char *argv[])
{
	file = fopen ("mouseoutput.txt", "w");
	glutInit(&argc,argv);
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
	glutInitWindowSize (500, 500);
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("mouse test");
	init2D(0.0,0.0,0.0);
	glutDisplayFunc(display);
	glutPassiveMotionFunc(mouse);
	glutMainLoop();
	fclose(file);
}