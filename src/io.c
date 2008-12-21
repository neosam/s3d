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

#include <curl/curl.h>

#include <SDL.h>
#include <SDL_net.h>

#include "io.h"
#include "misc.h"
#include "manager.h"
#include "compiler.h"
#include "exception.h"

char *ioerr;
int tcplistening = 0;
char *curlBuffer;
int curlBufferSize;

struct handleURLItem handleURLList[] = {{"file", handleFILE},
					{"s3ds", handleS3DS},
					{"s3d", handleS3D},
					{"http", handleHTTP},
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
		THROWS(-1, "Not a valid domain");
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

	THROWS(-1, "Protokoll not found");
	return NULL;
}

int curlGet(void *data, size_t size, size_t size_nmemt, void *userp)
{
	int curlBufferOldSize = curlBufferSize;
	if (curlBufferSize == 0) {
		curlBufferSize = size_nmemt;
		curlBuffer = malloc(sizeof(char) * size_nmemt);
	}
	else {
		curlBufferSize += size_nmemt;
		curlBuffer = (char*) realloc(curlBuffer, 
				sizeof(sizeof(char) * curlBufferSize)); 
	}

	sprintf(curlBuffer + curlBufferOldSize, "%s", data);

	return size_nmemt;
}

char *handleHTTP(char *server, int port, char *rest, int *size)
{
	char *res;
	char *url = (char*) malloc(sizeof(char) * 256);
	char errorCode[256];
	CURL *handle = curl_easy_init();

	sprintf(url, "http://%s/%s", server, rest);

	curl_easy_setopt(handle, CURLOPT_URL, url);
	curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, curlGet);
	curl_easy_setopt(handle, CURLOPT_ERRORBUFFER, errorCode);

	if (curl_easy_perform(handle) != 0)
		fprintf(stderr, "Error: %s %s\n", url, errorCode);

	*size = curlBufferSize;
	return curlBuffer;
}

char *handleS3DS(char *server, int port, char *rest, int *size)
{
	char *res;

	THROWIF(port != -1, -1, "Sorry s3d just works with port -1");
	THROWIF(strcmp(server, "localhost"), -1,
		"Sorry, s3d just with localhost");

	initManager();
	res = getCurrentSource(rest);
	quitManager();
	*size = strlen(res);

	return res;
}

char *handleFILE(char *server, int port, char *rest, int *size)
{
	FILE *file;
	char *res;

	THROWIF(port != -1 || strcmp(server, "localhost"), -1,
		"A file can just read at localhost, do not specify a host "
		"or port");

	file = fopen(rest, "r");
	res = MALLOCN(char, 4096);
	THROWIF(file == NULL, -1, "cannot open file");
	*size = fread(res, 1, 4096, file);

	return res;
}

char *handleS3D(char *server, int port, char *rest, int *size)
{
	char *res;

	THROWIF(port != -1, -1, "Sorry s3d just works with port -1");
	THROWIF(strcmp(server, "localhost"), -1,
		"Sorry, s3d just with localhost");

	initManager();
	res = getCurrent(rest);
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

int listenToCode(TCPsocket tcpclient, char *answer)
{
	int brakets = 0;
	int i;
	char *data = MALLOCN(char, 4096);
	int size = 0;

	for (;;) {
		answer += size;
		size = SDLNet_TCP_Recv(tcpclient, data, 4095);
		data[size] = '\0';
		memcpy(answer, data, size);
		for (i = 0; i < size; i++) {
			switch (answer[i]) {
			case '(':
				brakets++;
				break;
			case ')':
				brakets--;
				if (brakets == 0) {
					answer[i+1] = '\0';
					return 0;
				}
				if (brakets < 0)
					return -1;
				break;
			}
		}
	}
}

int _listenTCPClient(void *tcpclient)
{
	char *data = MALLOCN(char, 4096);
	char *answer;
	char *name;
	int size;

	while (1) {
		SDL_Delay(100);
		size = SDLNet_TCP_Recv(tcpclient, data, 4095);
		data[size] = '\0';
		if (data[0] == 'd') {
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
		if (data[0] == 'u') {
			char *stream;
			printf("update or insert in ascii\n");
			answer = MALLOCN(char, 4096);
			name = strtok(data+1, "\n\t\r");
			if (listenToCode(tcpclient, answer) == -1)
				fprintf(stderr, "Something went wrong\n");
			printf("Name: %s\nCode: %s\n", name, answer);
			stream = compile(answer);
			if (stream == NULL) {
				fprintf(stderr, "COMPILER ERROR: %s\n", 
					compilererr);
				break;
			}
			
			set(name, stream, answer, 0);
		}
	}
}

int _listenTCP(void *tcpsock)
{
	TCPsocket sock;

	while (1) {
		SDL_Delay(100);
		if ((sock = SDLNet_TCP_Accept(tcpsock)) != NULL) {
			SDL_CreateThread(_listenTCPClient, 
					 (void *)sock);
		}
	}
}

int listenTCP(int port)
{
	IPaddress ip;
	TCPsocket tcpsock;
	THROWIF(SDLNet_ResolveHost(&ip, NULL, port) == -1, -1, 
		"Could not resolve host");
	tcpsock = SDLNet_TCP_Open(&ip);
	THROWIF(!tcpsock, -1, "Could not open tcp listener");

	SDL_CreateThread(_listenTCP, (void *)tcpsock);
}
