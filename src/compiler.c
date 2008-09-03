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
#include <ossp/uuid.h>
#include <assert.h>

#include "compiler.h"
#include "sha1.h"
#include "misc.h"

void *compilererr;

char *skipSpaces(char *pc)
{
	while(*pc == ' ' || *pc == '\n' || *pc == '\t')
		if (*pc++ == '\0') {
			compilererr = "EOF or \\0 in file";
			return NULL;
		}
	return pc;
}

char *scanTagname(char **tagname, char *pc)
{
	int i;
	char *t;

	if (*pc++ != '(') {
		compilererr = "Expected (";
		return NULL;
	}
 
	if ((pc =skipSpaces(pc)) == NULL)
		return NULL;

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
	if (*++pc != '"') {
		compilererr = "Expected \"";
		return NULL;
	}
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

	if (*pc++ != '(') {
		compilererr = "Expected ( at parameter";
		return NULL;
	}

	for (;;) {
		if ((pc = skipSpaces(pc)) == NULL)
			return NULL;
		if (*pc == ')')
			return ++pc;
		if ((pc = scanParameter(param, pc)) == NULL)
			return NULL;
		param += 2;
	}
}

char *scanTag(char *pc, char **tagname, char ***parameter)
{
	if ((pc = scanTagname(tagname, pc)) == NULL)
		return NULL;
	if ((pc = skipSpaces(pc)) == NULL)
		return NULL;
	if ((pc = scanParameterlist(parameter, pc)) == NULL)
	    return NULL;

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
	if (sscanf(parameter, "(%f %f %f)", res, res+1, res+2) != 3) {
		compilererr = "Expected float tripple";
		return NULL;
	}
	return res;
}

int writeTriToStream(char *stream, char *tagname, char **parameter)
{
	char *v0 = lookupList(parameter, "v0"),
		*v1 = lookupList(parameter, "v1"),
		*v2 = lookupList(parameter, "v2");
	float *d0, *d1, *d2;
	const type = 1;

	if (v0 == NULL || v1 == NULL || v2 == NULL) {
		compilererr = "Not all parameter defined";
		return -1;
	}

	d0 = parseV3f(v0);
	d1 = parseV3f(v1);
	d2 = parseV3f(v2);

	if (d0 == NULL || d1 == NULL || d2 == NULL)
		return -1;

	memcpy(stream, &type, 4);
	memcpy(stream+4, d0, 12);
	memcpy(stream+16, d1, 12);
	memcpy(stream+28, d2, 12);

	return 40;
}

int writeTag(char *stream, char *tagname, char **parameter)
{
	if (strcmp(tagname, "tri") == 0)
		return writeTriToStream(stream, tagname, parameter);
	compilererr = "Tagname not found";
	return -1;
}

int writeData(char *stream, char *code)
{
	char *p = stream;
	char *pc = code;
	char *tagname;
	char **parameter;
	int size;

	if ((pc = scanTag(pc, &tagname, &parameter)) == NULL) 
		return -1;

	if ((pc = skipSpaces(pc)) == NULL)
		return -2;

	if (*pc != ')') {
		compilererr = "Expected ) at end of file";
		return -3;
	}

	if ((size = writeTag(stream + 4, tagname, parameter)) < 0)
		return -4;
	
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
	if ((datasize = writeData(stream, code)) < 0)
		return NULL;

	return stream;
}
