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

#include "compiler.h"
#include "sha1.h"
#include "misc.h"


/*
 * The Head of the stream object:
 * | author-length | author-name | date | parent |
 * | 8 bit number  |    string   | 32bit| 20byte |
 */
int writeHead(char *stream, char *author, char *parent)
{
	int aleni = strlen(author);
	char *p = stream + 20; /* There is a sha1 before the head */
	unsigned int date = time(NULL);
	
	if (aleni > 256)
		return -1;
	
	/* Set author length */
	*(p++) = aleni;
	
	/* Set author */
	strcpy(p, author);
	p += aleni;

	/* Set date TODO: ENDIAN */
	memcpy(p, &date, 4);
	p += 4;

	/* Set parent */
	memcpy(p, parent, 20);

	/* Length = author length + author + date + parent */
	return 1 + aleni + 8 + 20;
}

int insertSha1(char *stream, int size)
{
	SHA_CTX sha;
	SHA1_Init(&sha);
	SHA1_Update(&sha, stream+20, size);
	SHA1_Final(stream, &sha);
}

char *compile(char *code, char *author, char *parent)
{
	char *stream = MALLOCN(char, 4096);
	int headsize, datasize;

	if ((headsize = writeHead(stream, author, parent)) < 0)
		return NULL;

	/* Compile data is coming soon */
	datasize = 0;

	insertSha1(stream, headsize + datasize);

	return stream;
}
