#include <stdio.h>

#include "mesh.h"

int main(int argc, char **argv)
{
	struct mesh *m = mesh_newTriangle();
	struct face *f = m->f[0];
	int i;

	for (i = 0; i < 3; i++) {
		printf("%f %f %f\n", f->v[i]->x,
				f->v[i]->y,
				f->v[i]->z);
	}

	return 0;
}
