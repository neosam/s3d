/*
 * File export.c
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
#include <iostream>

#include "manager.h"

using namespace std;

int main(int argc, char **argv)
{
	try {
		s3d::Manager manager;
		char *code;
		FILE *input;
		
		if (input == NULL) {
			fprintf(stderr, "Could not create file\n");
			return 1;
		}
		
		code = manager.getCurrentSource(argv[1]);
		
		input = fopen(argv[1], "w");
		fwrite(code, strlen(code), 1, input);
		
		return 0;
	}
	catch (char *error) {
		cout << error << endl;
		return 1;
	}
	catch (...) {
		cout << "Unknows error" << endl;
		return 1;
	}
}
