/*
 * File compiler.c
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

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "compiler.h"
#include "sha1.h"
#include "misc.h"
#include "exception.h"

void *compilererr;

char *skipSpaces(char *pc)
{
	while((*pc == ' ' || *pc == '\n' || *pc == '\t' || *pc == '\r') &&
			*pc != '\0')
		pc++;
//		THROWIF(*pc++ == '\0', -1, "EOF or \\0 in file");
	return pc;
}

char *scanTagname(char **tagname, char *pc)
{
	int i;
	char *t;

	THROWIF(*pc++ != '(', -1, "Expected (");
	pc = skipSpaces(pc);
	*tagname = MALLOCN(char, 256);
	t = *tagname;

	while (*pc != ' ' && *pc != '\n')
		*t++ = *pc++;
	*t = '\0';

	return pc;
} 

char *scanParameter(char **param, char *pc)
{
	char *name, *value;

	name = param[0] = MALLOCN(char, 256);
	value = param[1] = MALLOCN(char, 256);
	param[2] = NULL;

	/* scan name */
	while (*pc != '=')
		*name++ = *pc++;
	*name = '\0';
       
	/* Check for =" */
	THROWIF(*++pc != '"', -1, "Expected \"");
	pc++;

        /* scan value */
	while (*pc != '"') 
		*value++ = *pc++;
	*value='\0';

	return ++pc;
}

char *scanParameterlist(char ***parameter, char *pc)
{
	char **param;
	*parameter = MALLOCN(char *, 256);
	param = *parameter;

	*param = NULL;

	THROWIF(*pc++ != '(', -1, "Expected ( at parameter");

	for (;;) {
		pc = skipSpaces(pc);
		if (*pc == ')')
			return ++pc;
		pc = scanParameter(param, pc);
		param += 2;
	}
}

char *scanTag(char *pc, char **tagname, char ***parameter)
{
	pc = scanTagname(tagname, pc);
	pc = skipSpaces(pc);
	pc = scanParameterlist(parameter, pc);

	return pc;
}

char *lookupList(char **list, char *key)
{
	int i;
	for (i = 0; list[i] != NULL; i+=2) {
		if (strcmp(list[i], key) == 0) {
			return list[i+1];
		}
	}
	
	return NULL;
}

float *parseV3f(char *parameter)
{
	float *res = MALLOCN(float, 3);
	THROWIF(sscanf(parameter, "(%f %f %f)", res, res+1, res+2) != 3, -1,
		"Expected float tripple");
	return res;
}

int *parseIntList(char *parameter, int *size)
{
	char *p = parameter + 1;
	int *list = MALLOCN(int, 256);
	int pos = 0;
	
	while (*p != ')' && *p != '\0') {
		list[pos] = atoi(p);
		pos++;
		while (*p != ' ' && *p != '\n' && *p != '\t' && *p != '\0') p++;
		p = skipSpaces(p);
	}

	*size = pos;
	return list;
}

int writeTriToStream(char *stream, char *tagname, char **parameter)
{
	char *v0 = lookupList(parameter, "v0"),
		*v1 = lookupList(parameter, "v1"),
		*v2 = lookupList(parameter, "v2");
	float *d0, *d1, *d2;
	const type = 1;

	THROWIF(v0 == NULL || v1 == NULL || v2 == NULL, -1, 
		"Not all parameter defined");

	d0 = parseV3f(v0);
	d1 = parseV3f(v1);
	d2 = parseV3f(v2);

	memcpy(stream, &type, 4);
	memcpy(stream+4, d0, 12);
	memcpy(stream+16, d1, 12);
	memcpy(stream+28, d2, 12);

	free(d0);
	free(d1);
	free(d2);


	return 40;
}

int writeCubeToStream(char *stream, char *tagname, char **parameter)
{
	const type = 2;
	memcpy(stream, &type, 4);
	return 4;
}

int writeExtrudeToStream(char *stream, char *tagname, char **parameter)
{
	char *face = lookupList(parameter, "face");
	char *offset = lookupList(parameter, "offset");
	const type = 0x10;

	int iface;
	float *foffset;

	THROWIF(face == NULL || offset == NULL, -1,
			"Not all parameter defined");

	iface = atoi(face);
	foffset = parseV3f(offset);

	memcpy(stream, &type, 4);
	memcpy(stream + 4, &iface, 4);
       	memcpy(stream + 8, foffset, 12);

	free(foffset);

	return 20;
}

int writeExtrudenToStream(char *stream, char *tagname, char **parameter)
{
	char *face = lookupList(parameter, "face");
	char *offset = lookupList(parameter, "offset");
	const type = 0x11;
	int size;

	int *iface;
	float *foffset;

	THROWIF(face == NULL || offset == NULL, -1,
			"Not all parameter defined");

	iface = parseIntList(face, &size);
	foffset = parseV3f(offset);

	memcpy(stream, &type, 4);
	memcpy(stream + 4, &size, 4);
	memcpy(stream + 8, iface, size * 4);
	memcpy(stream + size * 4 + 8, foffset, 12);

	free(iface);
	free(foffset);

	return 20 + size * 4;
}

int writeTag(char *stream, char *tagname, char **parameter)
{
	/* Raw Mesh */
	if (strcmp(tagname, "tri") == 0)
		return writeTriToStream(stream, tagname, parameter);
	if (strcmp(tagname, "cube") == 0)
		return writeCubeToStream(stream, tagname, parameter);

	/* Mesh manipulation */
	if (strcmp(tagname, "extrude") == 0)
		return writeExtrudeToStream(stream, tagname, parameter);
	if (strcmp(tagname, "extruden") == 0)
		return writeExtrudenToStream(stream, tagname, parameter);
	THROWS(-1, "Tagname not found");
}

int writeData(char *stream, char *code)
{
	char *p = stream;
	char *pc = code;
	char *tagname;
	char **parameter;
	int size = 0;

	do {
		pc = scanTag(pc, &tagname, &parameter);
		pc = skipSpaces(pc);
		THROWIF(*pc != ')', -1,
				"Expected )");
		size += writeTag(stream + size + 4, tagname, parameter);
		pc++;
		pc = skipSpaces(pc);
	} while (*pc != '\0');

	size += 4;
	memcpy(stream, &size, 4);

	free(tagname);
	free(parameter);

	return size;
}

int getStreamsize(char *stream)
{
	int *b = (int*)stream;
	return *b;
}

char *compile(char *code)
{
	char *stream = MALLOCN(char, 4096);
	int headsize, datasize;
	char *id;

	code = skipSpaces(code);
	datasize = writeData(stream, code);

	return stream;
}
