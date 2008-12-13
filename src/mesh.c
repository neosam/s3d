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

	res->v = MALLOCN(struct vertex *, 3);
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
	mesh_addVertex(res, vertex_new(-1.0, -1.0, 0.0));
	mesh_addVertex(res, vertex_new( 1.0,  1.0, 0.0));
	mesh_addFace(res, face_new(res->v[0], res->v[1], res->v[2])); 

	return res;
}

void mesh_appendVertex(struct mesh *m, struct vertex *v,
		int index, int index2)
{
	mesh_addVertex(m, v);
	mesh_addFace(m, face_new(v, m->v[index], m->v[index2]));
}

void mesh_connectEdges(struct mesh *m, struct vertex *v0, struct vertex *v1,
		struct vertex *v0copy, struct vertex *v1copy)
{
	mesh_addFace(m, face_new(v1, v1copy, v0));
	mesh_addFace(m, face_new(v0, v1copy, v0copy));
}

void mesh_extrude(struct mesh *m, int face,
		double offsetX, double offsetY, double offsetZ)
{
	struct face *f = m->f[face];
	struct vertex **v0 = MALLOCN(struct vertex *, 3);
	struct vertex **v1 = MALLOCN(struct vertex *, 3);
	int i;
	
	for (i = 0; i < 3; i++) {
		v1[i] = vertex_new(f->v[i]->x + offsetX,
				f->v[i]->y + offsetY,
				f->v[i]->z + offsetZ);
		mesh_addVertex(m, v1[i]);

		/* Save old position and move the face */
		v0[i] = f->v[i];
		f->v[i] = v1[i];
	}

	for (i = 0; i < 3; i++) {
		mesh_connectEdges(m, v0[i], v0[(i+1)%3],
				v1[i], v1[(i+1)%3]);
	}
}

int mesh_getVertexFromPool(struct vertex **pool, int n, struct vertex *v)
{
	for (; n >= 0; --n) {
		if (pool[n] == v)
			return n;
	}
	return -1;
}


struct mesh_edge {
	struct vertex *v0, *v1;
	int noDraw;
};

void mesh_banLastFromEPool(struct mesh_edge **ePool, int n)
{
	struct mesh_edge *e = ePool[n-1];
	int i;

	for (i = 0; i < n - 1; i++) {
		if (ePool[i]->v0 == e->v0 && ePool[i]->v1 == e->v1 ||
			ePool[i]->v0 == e->v1 && ePool[i]->v1 == e->v0) {
			ePool[i]->noDraw = 1;
			e->noDraw = 1;
		}
	}
}

void mesh_extruden(struct mesh *m, int *face, int n,
		double offsetX, double offsetY, double offsetZ)
{
	struct face *f;
	struct vertex **v0 = MALLOCN(struct vertex *, 3*n);
	struct vertex **v1 = MALLOCN(struct vertex *, 3*n);
	struct mesh_edge **ePool = MALLOCN(struct mesh_edge *, 3*n);
	int i, j, index;
	int pos = 0, ePos = 0;

	for (j = 0; j < n; j++) {
		f = m->f[face[j]];
		for (i = 0; i < 3; i++) {
			index = mesh_getVertexFromPool(v0, pos, f->v[i]);
			v0[pos] = f->v[i];
			if (index < 0) {
				v1[pos] = vertex_new(f->v[i]->x + offsetX,
					f->v[i]->y + offsetY,
					f->v[i]->z + offsetZ);
				mesh_addVertex(m, v1[pos]);
				index = pos;

				pos++;
			}

			/* Save old position and move the face */
			f->v[i] = v1[index];
		}
		for (i = 0; i < 3; i++) {
			ePool[ePos] = MALLOC(struct mesh_edge);
			ePool[ePos]->v0 = f->v[i];
			ePool[ePos]->v1 = f->v[(i+1)%3];
			ePool[ePos]->noDraw = 0;
			ePos++;
			mesh_banLastFromEPool(ePool, ePos);
		}
	}

	for (i = 0; i < ePos; i++) {
		int index0 = mesh_getVertexFromPool(v1, pos, ePool[i]->v0), 
		    index1 = mesh_getVertexFromPool(v1, pos, ePool[i]->v1);
		if (!ePool[i]->noDraw)
			mesh_connectEdges(m, v0[index0], v0[index1],
					v1[index0], v1[index1]);
	}
}
