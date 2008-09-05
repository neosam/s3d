#ifndef IO_H
#define IO_H

extern const char *ioerr;

struct handleURLItem {
	char *protokoll;
	char *(*func)(char *server, int port, char *rest, int *size);
};
extern struct handleURLItem handleURLList[]; 

char *getURL(char *url, const char *defaultp, int *size);

char *handleFILE(char *server, int port, char *rest, int *size);
char *handleS3D(char *server, int port, char *rest, int *size);
char *handleS3DS(char *server, int port, char *rest, int *size);

int listenTCP(int port);

#endif /* IO_H */
