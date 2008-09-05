#include <stdio.h>

#include "msgqueue.h"
#include "global.h"
#include "errno.h"

int magic_key, msg;

void s3d_errorMessageQueueInit()
{
	switch (errno) {
	case EACCES:
		printf("Message queue exists but no permission\n");
		break;
	case EEXIST:
		printf("Message queue exists - can't create\n");
		break;
	case ENOENT:
		printf("Message queue doesn't exist\n");
		break;
	case ENOMEM:
		printf("System is out of memory\n");
		break;
	case ENOSPC:
		printf("Maximum number of message queues reached\n");
		break;
	}
}

int s3d_createMessageQueue()
{
	int key = MESSAGE_KEY;
	int exit = MESSAGE_KEY + 0x100;
	int res;

	do {
		res = msgget(key, IPC_CREAT | IPC_EXCL | 0660);
		s3d_errorMessageQueueInit();
		if (res == exit) return -1;
		key++;
	} while (res == -1);

	magic_key = key-1;
	msg = res;
	
	return 0;
}

int s3d_connectMessageQueue(int key)
{
	magic_key = key;
	s3d_errorMessageQueueInit();
	printf("key: 0x%x\n", key);
	msg = msgget(key, 0660);
}

