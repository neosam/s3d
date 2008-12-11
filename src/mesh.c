#include "mesh.h"
#include "misc.h"

struct vertex *vertex_new(double x, double y, double z)
{
	struct vertex *res = MALLOC(struct vertex);

	res->x = x;
	res->y = y;
	res->z = z;
	res->f = MALLOCN(struct face*, VERTEX_SIZE);
	res->faces = 0;
	res->max_faces = VERTEX_SIZE;

	return res;
}

void vertex_addFace(struct vertex *v, struct face *f)
{
	v->f[v->faces] = f;
	v->faces++;
	if (v->faces == v->max_faces) {
		v->max_faces += v->max_faces;
		v->f = (struct face **) realloc((void*) v->f, 
				sizeof(struct face*) * v->max_faces);
	}
}


struct face *face_new(struct vertex *v0, struct vertex *v1,
		struct vertex *v2)
{
	struct face *res = MALLOC(struct face);
	int i;

	res->v[0] = v0;
	res->v[1] = v1;
	res->v[2] = v2;

	for (i = 0; i < 3; i++)
		vertex_addFace(res->v[i], res);

	return res;
}


struct mesh *mesh_new()
{
	struct mesh *res = MALLOC(struct mesh);
	int i;

	res->f = MALLOCN(struct face *, MESH_FACES_SIZE);
	res->v = MALLOCN(struct vertex *, MESH_VERTEX_SIZE);
	res->faces = 0;
	res->vertices = 0;
	res->max_faces = MESH_FACES_SIZE;
	res->max_vertices = MESH_VERTEX_SIZE;

	return res;
}

void mesh_addFace(struct mesh *m, struct face *f)
{
	m->f[m->faces] = f;
	m->faces++;
	if (m->faces == m->max_faces) {
		m->max_faces *= 2;
		m->f = REALLOCN(struct face *, m->f, m->max_faces);
	}
}

void mesh_addVertex(struct mesh *m, struct vertex *v)
{
	m->v[m->vertices] = v;
	m->vertices++;
	if (m->vertices == m->max_vertices) {
		m->max_vertices *= 2;
		m->v = REALLOCN(struct vertex *, m->v, m->max_vertices);
	}
}

struct mesh *mesh_newTriangle()
{
	struct mesh *res = mesh_new();
	
	mesh_addVertex(res, vertex_new(-1.0,  1.0, 0.0));
	mesh_addVertex(res, vertex_new( 1.0,  1.0, 0.0));
	mesh_addVertex(res, vertex_new(-1.0, -1.0, 0.0));
	mesh_addFace(res, face_new(res->v[0], res->v[1], res->v[2]));

	return res;
}
