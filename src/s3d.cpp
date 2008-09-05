/*
 * File s3d.c
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

#include <SDL.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include "global.h"
#include "msgqueue.h"

pid_t displayID;

int quitProcesses()
{
	kill(displayID, SIGTERM);
       
	return 0;
}

void quit(int signal)
{
	quitProcesses();
	if (msgctl(msg, IPC_RMID, NULL) != 0)
		printf("Could not destroy message queue\n");
	exit(0);
}

int init()
{
	signal(SIGINT, quit);

	if (s3d_createMessageQueue() < 0)
		return 1;

	return 0;
}

int createDisplay()
{
	pid_t pid;
	char key[32];

	switch (pid = fork()) {
	case -1: return 1;
	case 0: 
		sprintf(key, "%x\n", magic_key);
		execlp("./s3d_display", "./n3d_display", "-m", key, NULL);
		break;
	default:
		displayID = pid;
		return 0;
	}
}

int createProcesses()
{
	if (createDisplay() != 0)
		return 1;
}

int handleProcesses()
{
	pid_t pid;
	int status;

	while (1) {
		printf("waiting\n");
		pid = wait(&status);
		printf("action\n");
		if (pid == displayID)
			quit(0);
	}
}

int main(int argc, char **argv)
{
	if (init() != 0)
		return 1;
	if (createProcesses() != 0)
		return 2;
	handleProcesses();

	return 0;
}
