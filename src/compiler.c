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
}

int writeData(char *stream, int headsize, char *code)
{
	char *p = stream + headsize + 20;

	strcpy(p, "DATA");

	return 4;
}

/* Reads the head (id) and return the code after ( */
char *checkCodeHeader(char *code, char **id)
{
	for (;;) {
		if (*code == '\0') return NULL;
		if (*code == '(') return code;
		code++;
	}
}

char *compile(char *code, char *author, char *parent)
{
	char *stream = MALLOCN(char, 4096);
	int headsize, datasize;
	char *id;

	if ((code = checkCodeHeader(code, &id)) == NULL)
		return NULL;

	if ((headsize = writeHead(stream, author, parent, NULL)) < 0)
		return NULL;

	if ((datasize = writeData(stream, headsize, code)) < 0)
		return NULL;

	insertSha1(stream+headsize, datasize);

	return stream;
}
