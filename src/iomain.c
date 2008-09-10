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
#include <string.h>

#include <SDL.h>
#include <SDL_net.h>

#include "io.h"
#include "manager.h"
#include "exception.h"

int main(int argc, char **argv)
{
	char *res;
	int size;

	TRY;
	if (strcmp(argv[1], "--serve") == 0) {
		int port = atoi(argv[2]);
		SDL_Init(SDL_INIT_TIMER | SDL_INIT_NOPARACHUTE);
		SDLNet_Init();
		initManager();
		listenTCP(port);
		scanf("%s", res);
		SDLNet_Quit();
		quitManager();
	}

	res = getURL(argv[1], "file", &size);
	fprintf(stderr, "%i\n", size);

	fwrite(res, size, 1, stdout);

	CATCH;
	fprintf(stderr, "%s\n", excmsg);
	TRYEND;

	return 0;
}
