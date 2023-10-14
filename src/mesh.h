#pragma once

#include "vector.h"
#include "triangle.h"
#include "upng.h"


////////////////////////////////////////////////////////////////////////
// Defines a struct for dynamic sized meshes with an array of vertices and faces
////////////////////////////////////////////////////////////////////////
typedef struct
{
    vec3_t *vertices; // Dynamic array of vertices
    face_t *faces;    // Dynamic array of faces
    upng_t* texture; // Mesh png texture pointer
    vec3_t rotation;
    vec3_t scale;
    vec3_t translation;

} mesh_t;

void load_mesh_obj_data(mesh_t* mesh, char* filename);
void load_mesh_png_data(mesh_t* mesh, char* filename);
void load_mesh(char* obj_filename, char* png_filename, vec3_t scale, vec3_t translation, vec3_t rotation);
int get_num_meshes(void);
mesh_t* get_mesh(int index);
void free_meshes(void);