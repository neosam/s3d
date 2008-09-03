#ifndef IO_H
#define IO_H

extern char *ioerr;

struct handleURLItem {
	char *protokoll;
	char *(*func)(char *server, int port, char *rest);
};
extern struct handleURLItem handleURLList[]; 

char *getURL(char *url, char *defaultp);

char *handleFILE(char *server, int port, char *rest);
char *handleS3DS(char *server, int port, char *rest);

#endif /* IO_H */
