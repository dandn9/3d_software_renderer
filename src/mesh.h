#pragma once

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
#define N_CUBE_FACES (6 * 2) // 2 triangle per face

extern vec3_t cube_vertices[N_CUBE_VERTICES];
extern face_t cube_faces[N_CUBE_FACES];
extern void load_cube_mesh_data(void);

////////////////////////////////////////////////////////////////////////
// Defines a struct for dynamic sized meshes with an array of vertices and faces
////////////////////////////////////////////////////////////////////////
typedef struct
{
    vec3_t *vertices; // Dynamic array of vertices
    face_t *faces;    // Dynamic array of faces
    vec3_t rotation;
} mesh_t;

extern mesh_t mesh;