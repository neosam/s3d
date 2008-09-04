/*
 * File display.c
 *
 * s3d is the legal property of Simon Goller (neosam@gmail.com).
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/*
 * Checker texture is from the OpenGL Redbook
 */

#include <string.h>
#include <stdio.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include "msgqueue.h"

int done = 0;
char *disperr;

#define checkImageWidth 64
#define checkImageHeight 64

static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

static GLuint texName;

void makeCheckImage(void)
{
   int i, j, c;
    
   for (i = 0; i < checkImageHeight; i++) {
      for (j = 0; j < checkImageWidth; j++) {
         c = ((((i&0x8)==0)^((j&0x8))==0))*255;
         checkImage[i][j][0] = (GLubyte) c;
         checkImage[i][j][1] = (GLubyte) c;
         checkImage[i][j][2] = (GLubyte) c;
         checkImage[i][j][3] = (GLubyte) 255;
      }
   }
}



void quit(int signal)
{
	printf("s3d_display: start exiting\n");
	done = 1;
}

void parseArguments(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m")) {
			s3d_connectMessageQueue(strtol(argv[i+1], NULL, 16));
			i++;
		}
	}
}

int updateDisplay(int width, int height)
{
	if (SDL_SetVideoMode(width, height, 
			     16, SDL_OPENGL | SDL_RESIZABLE) == NULL)
	{
		disperr = "Could not create window";
		return -1;
	}
	SDL_WM_SetCaption("S3D", NULL);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLdouble)width/(GLdouble)height, .1, 100.0);
	glMatrixMode(GL_MODELVIEW);
	glClearColor(.3, .3, 1.0, 0.0);
	gluLookAt(0, 0, -2,
		  0, 0, 0,
		  0, 1, 0);

	return 0;
}

int initDisplay()
{
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		disperr = "Could not init SDL";
		return -1;
	}

	if (updateDisplay(640, 480) != 0)
		return -1;


	glEnable(GL_TEXTURE_2D);

	/* All from the redbook gg */
	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkImageWidth, 
		     checkImageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
		     checkImage);
	
	return 0;
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_TRIANGLES);
	glTexCoord2f(.5, 0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1, 1); glVertex3f(1.0, -1.0, 0.0);
	glTexCoord2f(0, 1); glVertex3f(-1.0, -1.0, 0.0);
	glEnd();
	SDL_GL_SwapBuffers();
}

int main(int argc, char **argv)
{
	SDL_Event event;

	signal(SIGTERM, quit);

	if (initDisplay() != 0) {
		fprintf(stderr, "%s\n", disperr);
		return 0;
	}

	parseArguments(argc, argv);

	while (!done) {
		SDL_Delay(1000/30);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = 1;
				break;
			case SDL_VIDEORESIZE:
				updateDisplay(event.resize.w, event.resize.h);
				break;
			}
		}
		display();
	}

	printf("s3d-display: quit\n");

	return 0;
}
