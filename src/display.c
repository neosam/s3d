/*
 * File display.c
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
#include "msgqueue.h"

int done = 0;

void quit(int signal)
{
	printf("s3d_display: start exiting\n");
	done = 1;
}

int main(int argc, char **argv)
{
	SDL_Event event;

	signal(SIGTERM, quit);

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(640, 480, 16, SDL_OPENGL);

	printf("%s\n", argv[1]);

	s3d_connectMessageQueue(strtol(argv[1], NULL, 16));

	while (!done) {
		SDL_Delay(1000/30);
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_QUIT:
				done = 1;
			}
		}
	}

	printf("s3d-display: quit\n");

	return 0;
}
