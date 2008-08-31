/*
 * File compiler.c
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
#include "compiler.h"

char source[] = ";550e8400-e29b-11d4-a716-446655440000 "
	"(tri (v0=\"(0, 1, 0)\" v1=\"(-1, -1, 0)\" v2=\"(1, -1, 0)\"))";

int main(int argc, char **argv)
{
	char *stream;
	char *code = MALLOCN(char, 4096);
	FILE *input = fopen(argv[3], "r");
	
	fread(code, 4096, 1, input);

	stream = compile(code, argv[1], argv[2]);
	if (stream == NULL)
		printf("ERROR");

	fwrite(code, 4096, 1, stdout);
	fflush(stdout);

	return 0;
}
