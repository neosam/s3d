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

#include <SDL.h>
#include <SDL_net.h>

#include "io.h"
#include "misc.h"
#include "manager.h"

char *ioerr;
int tcplistening = 0;

struct handleURLItem handleURLList[] = {{"file", handleFILE},
					{"s3ds", handleS3DS},
					{"s3d", handleS3D},
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

char *handleURL(char *protokoll, char *server, int port, char *rest, int *size)
{
	struct handleURLItem *p = handleURLList;
	while (p->protokoll != NULL) {
		if (strcmp(p->protokoll, protokoll) == 0)
			return p->func(server, port, rest, size);
		p++;
	}

	ioerr = "Protokoll not found";
	return NULL;
}

char *handleS3DS(char *server, int port, char *rest, int *size)
{
	char *res;

	if (initManager() != 0) {
		ioerr = managererr;
		return NULL;
	}
	
	if (port != -1 || strcmp(server, "localhost") != 0) {
		ioerr = "Sorry s3ds just works for localhost at the moment";
		return NULL;
	}
	res = getCurrentSource(rest);
	if (res == NULL) {
		ioerr = managererr;
		return NULL;
	}

	quitManager();

	*size = strlen(res);

	return res;
}

char *handleFILE(char *server, int port, char *rest, int *size)
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
	*size = fread(res, 1, 4096, file);

	return res;
}

char *handleS3D(char *server, int port, char *rest, int *size)
{
	char *res;

	if (initManager() != 0) {
		ioerr = managererr;
		return NULL;
	}
	
	if (port != -1 || strcmp(server, "localhost") != 0) {
		ioerr = "Sorry s3ds just works for localhost at the moment";
		return NULL;
	}
	res = getCurrent(rest);
	if (res == NULL) {
		ioerr = managererr;
		return NULL;
	}

	quitManager();

	*size = *((int *)res);

	return res;
}


char *getURL(char *url, char *defaultp, int *size)
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

	return handleURL(protokoll, server, port, rest, size);
}

void _listenTCPClient(void *tcpclient)
{
	char *data = MALLOCN(char, 4096);
	char *answer;
	int size;

	while (1) {
		SDL_Delay(100);
		size = SDLNet_TCP_Recv(tcpclient, data, 4095);
		data[size] = '\0';
		if (data[0] == 'g') {
			data = strtok(data, " \n\t\r");
			answer = getURL(data+1, "s3ds", &size);
			if (answer == NULL) {
				answer = ioerr;
				size = strlen(answer);
			}
			SDLNet_TCP_Send(tcpclient, answer, size);
			if (answer != ioerr)
				free(answer);
		}
	}
}

void _listenTCP(void *tcpsock)
{
	TCPsocket sock;

	while (1) {
		SDL_Delay(100);
		if ((sock = SDLNet_TCP_Accept(tcpsock)) != NULL) {
			SDL_CreateThread(_listenTCPClient, (void *)sock);
		}
	}
}

int listenTCP(int port)
{
	IPaddress ip;
	TCPsocket tcpsock;
	if (SDLNet_ResolveHost(&ip, NULL, port) == -1) {
		ioerr = "Could not resolve host";
		return -1;
	}
       
	tcpsock = SDLNet_TCP_Open(&ip);
	if (!tcpsock) {
		ioerr = "Could not open tcp listener";
		return -1;
	}

	SDL_CreateThread(_listenTCP, (void *)tcpsock);
}
