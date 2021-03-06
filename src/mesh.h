#ifndef MESH_H
#define MESH_H

#define VERTEX_SIZE 4
#define MESH_VERTEX_SIZE 4
#define MESH_FACES_SIZE 4

struct vertex {
	double x, y, z;
	double tx, ty; /* Texture */
	struct face **f;
	unsigned int faces;
	unsigned int max_faces;
};

struct face {
	struct vertex **v;
	double x, y, z; /* Center */
};

struct mesh {
	struct face **f;
	struct vertex **v;
	unsigned int faces;
	unsigned int max_faces;
	unsigned int vertices;
	unsigned int max_vertices;
};

/*
 * Vertex functions
 */

struct vertex *vertex_new(double x, double y, double z);
void vertex_addFace(struct vertex *v, struct face *f);


/*
 * Face functions
 */

struct face *face_new(struct vertex *v0, 
		struct vertex *v1, 
		struct vertex *v2);

/*
 * Mesh functions
 */

struct mesh *mesh_new();
void mesh_addFace(struct mesh *m, struct face *f);
void mesh_addVertex(struct mesh *m, struct vertex *v);

struct mesh *mesh_newTriangle();
struct mesh *mesh_newCube();
void mesh_appendVertex(struct mesh *m, struct vertex *v,
		int index, int index2);
void mesh_extrude(struct mesh *m, int face,
		double offsetX, double offsetY, double offsetZ);
void mesh_extruden(struct mesh *m, int *face, int n,
		double offsetX, double offsetY, double offsetZ);

void mesh_translateFaces(struct mesh *m, int *face, int n,
		double x, double y, double z);
void mesh_scaleFaces(struct mesh *m, int *face, int n,
		double x, double y, double z);
void mesh_rotateFaces(struct mesh *m, int *face, int n,
		double a, double x, double y, double z);

#endif

