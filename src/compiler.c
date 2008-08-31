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

#include "compiler.h"
#include "sha1.h"
#include "misc.h"

int insertID(char *stream, char *id)
{
	/* If there is no id, I will create a new one */
	if (id == NULL) {
		uuid_t *uuid;
		uuid_create(&uuid);
		uuid_make(uuid, UUID_MAKE_V1);
		uuid_export(uuid, UUID_FMT_BIN, (void **)&id, NULL);
		uuid_destroy(uuid);
	}

	memcpy(stream, id, 16);
}


/*
 * The Head of the stream object:
 * | uuid | date | parent | author-length | author-name |
 * |16byte| 32bit| 20byte | 8 bit number  |    string   |
 * | object id   |        |      author information     |
 * |                     HEAD                           |
 */
int writeHead(char *stream, char *author, char *parent, char *id)
{
	int aleni = strlen(author);
	char *p = stream + 16; /* There is a uuid before the head */
	unsigned int date = time(NULL);
	
	if (aleni > 256)
		return -1;
	
	insertID(stream, id);

	/* Set date TODO: ENDIAN */
	memcpy(p, &date, 4);
	p += 4;

	/* Set parent */
	memcpy(p, parent, 20);
	p += 20;

	/* Set author length */
	*(p++) = aleni;
	
	/* Set author */
	strcpy(p, author);
	p += aleni;

	/* Length = id + date + parent + autor-length + autor-name */
	return 41 + aleni;
}

int insertSha1(char *stream, int size)
{
	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha, stream+20, size);
	SHA1_Final(stream, &sha);
	return 0;
}

char *skipSpaces(char *pc)
{
	while(*pc == ' ' || *pc == '\n' || *pc == '\t')
		if (*pc++ == '\0')
			return NULL;
	return pc;
}

char *scanTagname(char **tagname, char *pc)
{
	int i;
	char *t;

	if (*pc++ != '(')
		return NULL;
 
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
	if (*++pc != '"')
		return NULL;
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

	if (*pc++ != '(')
		return NULL;

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

int writeTriToStream(char *stream, char *tagname, char **parameter)
{
	char *v0 = lookupList(parameter, "v0"),
		*v1 = lookupList(parameter, "v1"),
		*v2 = lookupList(parameter, "v2");
	float d0, d1, d2;
	const type = 1;

	if (v0 == NULL || v1 == NULL || v2 == NULL)
		return -1;

	d0 = strtod(v0, NULL);
	d1 = strtod(v1, NULL);
	d2 = strtod(v2, NULL);

	memcpy(stream, &type, 4);
	memcpy(stream+4, &d0, 4);
	memcpy(stream+8, &d1, 4);
	memcpy(stream+12, &d2, 4);

	return 16;
}

int writeTag(char *stream, char *tagname, char **parameter)
{
	if (strcmp(tagname, "tri") == 0)
		return writeTriToStream(stream, tagname, parameter);
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

	if (*pc != ')')
		return -3;

	if ((size = writeTag(stream + 4, tagname, parameter)) < 0)
		return -4;
	
	memcpy(stream, &size, 4);

	free(tagname);
	free(parameter);

	return size;
}

void parseID(char *code, char **id)
{
	uuid_t *uuid;
	
	uuid_create(&uuid);
	uuid_import(uuid, UUID_FMT_STR, (void *)code, 36);
	uuid_export(uuid, UUID_FMT_BIN, (void **)id, NULL);
	uuid_destroy(uuid);
}

/* Reads the head (id) and return the code after ( */
char *checkCodeHeader(char *code, char **id)
{
	if (*code==';')
		parseID(++code, id);
	else
		*id = NULL;

	for (;;) {
		if (*code == '\0') return NULL;
		if (*code == '(') return code;
		code++;
	}

	return code;
}

char *compile(char *code, char *author, char *parent)
{
	char *stream = MALLOCN(char, 4096);
	int headsize, datasize;
	char *id;

	if ((code = checkCodeHeader(code, &id)) == NULL)
		return NULL;

	if (id == NULL)
		fprintf(stderr, "No ID found\n");

	if ((headsize = writeHead(stream, author, parent, id)) < 0)
		return NULL;

	if ((datasize = writeData(stream + headsize + 20, code)) < 0)
		return NULL;

	insertSha1(stream+headsize, datasize);

	return stream;
}
