#pragma once

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // 2 triangle per face

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];
void load_cube_mesh_data(void);
void load_obj_file_data(char *filename);

////////////////////////////////////////////////////////////////////////
// Defines a struct for dynamic sized meshes with an array of vertices and faces
////////////////////////////////////////////////////////////////////////
typedef struct
{
    vec3_t *vertices; // Dynamic array of vertices
    face_t *faces;    // Dynamic array of faces
    vec3_t rotation;
    vec3_t scale;
    vec3_t translation;

} mesh_t;

extern mesh_t mesh;