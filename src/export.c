#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "manager.h"


int main(int argc, char **argv)
{
	char *code;
	FILE *input;

	if (input == NULL) {
		fprintf(stderr, "Could not create file\n");
		return 1;
	}

	if (initManager() != 0) {
		fprintf(stderr, "MANAGER ERROR: %s\n", managererr);
		return 1;
	}

	if ((code = get(argv[1])) == NULL) {
		fprintf(stderr, "MANAGER ERROR: %s\n", managererr);
		return 1;
	}

	quitManager();
	
	input = fopen(argv[1], "w");
	fwrite(code, strlen(code), 1, input);

	return 0;
}
