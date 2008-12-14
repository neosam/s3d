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
#include "misc.h"
#include "mesh.h"
#include "manager.h"

int done = 0;
char *disperr;
char *obj;
double rotate = 0.0;

#define checkImageWidth 64
#define checkImageHeight 64

static GLubyte checkImage[checkImageHeight][checkImageWidth][4];

static GLuint texName;

char *createDataFromMesh(struct mesh *m);

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

char *parseStream(char *stream)
{
	int *istream = (int*) stream;
	struct mesh *m;
	int max = *((int*) stream);
	int pos = 4;

	istream++;

	while (pos < max) {
		fprintf(stderr, "Current istream: %i\n", *istream);
		switch (*istream) {
		case 2:
			m = mesh_newCube();
			istream++;
			pos += 4;
			break;
		case 0x10:
			istream++;
			mesh_extrude(m, *istream, *((float*)(istream+1)),
					*((float*)(istream+2)), 
					*((float*)(istream+3)));
			istream += 4;
			pos += 20;
			break;
		default:
			fprintf(stderr, "Error in stream: %i\n", *istream);
			return "";
		}
	}

	return createDataFromMesh(m);
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
		if (strcmp(argv[i], "-m") == 0) {
			s3d_connectMessageQueue(strtol(argv[i+1], NULL, 16));
			i++;
		}
		if (strcmp(argv[i], "-d") == 0) {
			obj = parseStream(getCurrent(argv[i+1]));
			i+=2;
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
	gluLookAt(0, 0, 2,
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

	if (updateDisplay(320, 240) != 0)
		return -1;


	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);

	/* All from the redbook gg */
	makeCheckImage();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glGenTextures(1, &texName);
	glBindTexture(GL_TEXTURE_2D, texName);
	
	/*glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
			GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			GL_LINEAR);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, checkImageWidth, 
		     checkImageHeight, GL_RGBA, GL_UNSIGNED_BYTE, 
		     checkImage);
	glEnable(GL_CULL_FACE);

	return 0;
}

void drawObject(char *obj)
{
	GLuint objlength = *((GLuint *)obj);
	obj += 4;

	while (objlength-- != 0) {
		glBindTexture(GL_TEXTURE_2D, *((GLuint *)obj));
		obj += 4;
		glInterleavedArrays(GL_T2F_N3F_V3F, 0, obj + 4);
		glDrawArrays(GL_TRIANGLES, 0, *((GLuint *) obj));
	}
}

void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	/*glBegin(GL_TRIANGLES);
	glTexCoord2f(.5, 0); glVertex3f(0.0, 1.0, 0.0);
	glTexCoord2f(1, 1); glVertex3f(1.0, -1.0, 0.0);
	glTexCoord2f(0, 1); glVertex3f(-1.0, -1.0, 0.0);
	glEnd();*/
	glLoadIdentity();
	rotate += 0.5;
	gluLookAt(0.0, 3.0, 6.0,
			0.0, 0.0, 0.0,
			0.0, 1.0, 0.0);
	glRotatef(rotate, 0.0, 1.0, 0.0);
	drawObject(obj);
	SDL_GL_SwapBuffers();
}

void setVertex(float *vertex,
	       float x, float y, float z,
	       float nx, float ny, float nz,
	       float tx, float ty)
{
	vertex[5] = x;
	vertex[6] = y;
	vertex[7] = z;
	vertex[2] = nx;
	vertex[3] = ny;
	vertex[4] = nz;
	vertex[0] = tx;
	vertex[1] = ty;
}

/*
 * | Parts | Texture | Size (faces*3) | Data | Texture | Size | Data |
 * | Parts |           Part 1                 |        Part 2         |
 */

char *createDataFromMesh(struct mesh *mesh)
{
	char *obj = MALLOCN(char, mesh->faces * 96 + 12);
	int *iobj = (int *)obj;
	float *fobj = (float *)obj;
	FILE *file = fopen("testout", "w");
	int i;

	*iobj = 1;
	iobj += 1;
	*iobj = 1;
	iobj += 1;
	*iobj = mesh->faces * 3;

	fobj += 3;

	for (i = 0; i < mesh->faces; i++) {
		setVertex(fobj, mesh->f[i]->v[0]->x,
				mesh->f[i]->v[0]->y,
				mesh->f[i]->v[0]->z,
				0, 0, 0,
				mesh->f[i]->v[0]->tx, 
				mesh->f[i]->v[0]->ty);
		fobj += 8;
		setVertex(fobj, mesh->f[i]->v[1]->x,
				mesh->f[i]->v[1]->y,
				mesh->f[i]->v[1]->z,
				0, 0, 0,
				mesh->f[i]->v[1]->tx,
				mesh->f[i]->v[1]->ty);
		fobj += 8;
		setVertex(fobj, mesh->f[i]->v[2]->x,
				mesh->f[i]->v[2]->y,
				mesh->f[i]->v[2]->z,
				0, 0, 0,
				mesh->f[i]->v[2]->tx,
				mesh->f[i]->v[2]->ty);
		fobj += 8;
	}

	fwrite(obj, 1, mesh->faces * 96 + 12, file);
	fflush(file);
	fclose(file);
	return obj;
}

char *createTriangle()
{
	char *obj = MALLOCN(char, 108);
	int *iobj = (int *)obj;
	float *fobj = (float *)obj;
	FILE *file = fopen("testout", "w");

	*iobj = 1;
	iobj += 1;
	*iobj = 1;
	iobj += 1;
	*iobj = 96;

	fobj += 3;

	setVertex(fobj, 0, 1, 0,
		  0, 0, 1,
		  .5, 0);
	fobj += 8;
	setVertex(fobj, 1, -1, 0,
		  0, 0, 1,
		  1, 1);
	fobj += 8;
	setVertex(fobj, -1, -1, 0,
		  0, 0, 1,
		  0, 1);

	fwrite(obj, 1, 108, file);
	fflush(file);
	fclose(file);
	return obj;
}

int main(int argc, char **argv)
{
	SDL_Event event;
/*	struct mesh *mesh = mesh_newCube();
	int faces[] = {0, 1};
	mesh_rotateFaces(mesh, faces, 2, 3.1415/4, 1, 0, 0);

	int faces2[] = {2, 3};
	int faces3[] = {4, 5};
	int faces4[] = {6, 7};
	int faces5[] = {8, 9};
	mesh_appendVertex(mesh, vertex_new(1.0, -1.0, 0.0),
			2, 1);
	mesh_extruden(mesh, faces, 2, 0, 0, 1);
	mesh_extruden(mesh, faces, 2, 1, 1, 1);
	mesh_extruden(mesh, faces, 2, 0, 0, 1);
	mesh_extruden(mesh, faces2, 2, -1, 0, 0);
	mesh_extruden(mesh, faces3, 2, 0, 1, 0);
	mesh_extruden(mesh, faces4, 2, 1, 0, 0);
	mesh_extruden(mesh, faces5, 2, 0, -1, 0);
	mesh_addFace(mesh, face_new(mesh->v[0], mesh->v[2], mesh->v[1]));
	mesh_addFace(mesh, face_new(mesh->v[1], mesh->v[2], mesh->v[3]));*/

	signal(SIGTERM, quit);

	initManager();

	if (initDisplay() != 0) {
		fprintf(stderr, "%s\n", disperr);
		return 0;
	}

	parseArguments(argc, argv);

//	obj = createDataFromMesh(mesh);

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
