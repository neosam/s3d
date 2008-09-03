/*
 * File manager.c
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
#include <stdlib.h>
#include <string.h>

#include "io.h"
#include "misc.h"

char *ioerr;

struct handleURLItem handleURLList[] = {{"file", handleFILE},
					{NULL, NULL}};

int splitProtokoll(char *url, char **protokoll, char **rest)
{
	*protokoll = strtok(url, ":");
	*rest = strtok(NULL, "");

	if (*rest == NULL) {
		*rest = *protokoll;
		*protokoll = NULL;
	}
	else if ((*rest)[0] != (*rest)[1] || (*rest)[0] != '/') {
		ioerr = "Not a valid domain";
		return 1;
	} else {
		*rest += 2;
	}

	return 0;
}

char *splitServer(char *url, char **server, int *port, char **rest)
{
	char *strport;

	*server = strtok(url, "/");
	*rest = strtok(NULL, "");
	
	if (*rest == NULL) {
		*rest = *server;
		*server = "localhost";
		*port = -1;
		return 0;
	}

	*server = strtok(*server, ":");
	strport = strtok(NULL, "");

	if (strport == NULL)
		*port = -1;
	else
		*port = atoi(strport);

	return 0;
}

char *handleURL(char *protokoll, char *server, int port, char *rest)
{
	struct handleURLItem *p = handleURLList;
	while (p->protokoll != NULL) {
		if (strcmp(p->protokoll, protokoll) == 0)
			return p->func(server, port, rest);
		p++;
	}

	ioerr = "Protokoll not found";
	return NULL;
}

char *handleFILE(char *server, int port, char *rest)
{
	FILE *file;
	char *res;

	if (port != -1 || strcmp(server, "localhost")) {
		ioerr = "A file can just read at localhost, do not specify a host or port";
		return NULL;
	}

	file = fopen(rest, "r");
	res = MALLOCN(char, 4096);
	if (file == NULL) {
		ioerr = "cannot open file";
		return NULL;
	}
	fread(res, 4096, 1, file);
	return res;
}

char *getURL(char *url, char *defaultp)
{
	char *protokoll;
	char *server;
	char *rest;
	int port;

	if (splitProtokoll(url, &protokoll, &rest) != 0)
		return NULL;

	if (protokoll == NULL)
		protokoll = defaultp;

	if (splitServer(rest, &server, &port, &rest) != 0)
		return NULL;

	fprintf(stderr, "Protokoll: %s\n", protokoll);
	fprintf(stderr, "Server: %s\n", server);
	fprintf(stderr, "Port: %i\n", port);
	fprintf(stderr, "Rest: %s\n", rest);

	return handleURL(protokoll, server, port, rest);
}
