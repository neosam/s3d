/*
 * File import.c
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
#include "manager.h"
#include "compiler.h"
#include "misc.h"
#include "exception.h"

int main(int argc, char **argv)
{
	char *code = MALLOCN(char, 4096);
	char *stream;
	FILE *input = fopen(argv[1], "r");

	TRY;
	initManager();

	fread(code, 4096, 1, input);
	stream = compile(code);
	if (stream == NULL) {
		fprintf(stderr, "COMPILER ERROR: %s\n", compilererr);
		return 1;
	}

	set(argv[1], stream, code, 0);
	quitManager();

	CATCH;
	fprintf(stderr, "ERROR: %s\n", excmsg);
	TRYEND;
}
