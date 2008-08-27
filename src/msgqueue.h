#ifndef MSGQUEUE_H
#define MSGQUEUE_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/signal.h>
#include "global.h"

extern int magic_key, msg;

int s3d_createMessageQueue();
int s3d_connectMessageQueue(int magic_key);

#endif
