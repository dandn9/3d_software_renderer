#include "mesh.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NUM_MESHES 10

static mesh_t meshes[MAX_NUM_MESHES];
static int mesh_count = 0;

void load_mesh(char *obj_filename, char *png_filename, vec3_t scale, vec3_t translation, vec3_t rotation)
{
    load_mesh_obj_data(&meshes[mesh_count], obj_filename);
    load_mesh_png_data(&meshes[mesh_count], png_filename);

    meshes[mesh_count].scale = scale;
    meshes[mesh_count].translation = translation;
    meshes[mesh_count].rotation = rotation;

    mesh_count++;
}

void load_mesh_png_data(mesh_t* mesh, char* filename)
{
    upng_t *png_image = upng_new_from_file(filename);
    if (png_image != NULL)
    {
        upng_decode(png_image);
        if (upng_get_error(png_image) == UPNG_EOK)
        {
            mesh->texture = png_image;
        }
    }
}

mesh_t *get_mesh(int index)
{
    return &meshes[index];
}
void free_meshes(void)
{

    for (int i = 0; i < mesh_count; i++)
    {
        upng_free(meshes[i].texture);
        array_free(meshes[i].faces);
        array_free(meshes[i].vertices);
    }
}
int get_num_meshes(void)
{
    return mesh_count;
}
void load_mesh_obj_data(mesh_t *mesh, char *filename)
{
    FILE *file;
    file = fopen(filename, "r");
    char line[1024];

    tex2_t *texcoords = NULL;

    while (fgets(line, 1024, file))
    {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0)
        {
            vec3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh->vertices, vertex);
        }
        // Texture coordinate information
        if (strncmp(line, "vt ", 3) == 0)
        {
            tex2_t texcoord;
            sscanf(line, "vt %f %f", &texcoord.u, &texcoord.v);
            array_push(texcoords, texcoord);
        }
        // Face information
        if (strncmp(line, "f ", 2) == 0)
        {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(
                line, "f %d/%d/%d %d/%d/%d %d/%d/%d",
                &vertex_indices[0], &texture_indices[0], &normal_indices[0],
                &vertex_indices[1], &texture_indices[1], &normal_indices[1],
                &vertex_indices[2], &texture_indices[2], &normal_indices[2]);
            face_t face = {
                .a = vertex_indices[0] - 1,
                .b = vertex_indices[1] - 1,
                .c = vertex_indices[2] - 1,
                .a_uv = texcoords[texture_indices[0] - 1],
                .b_uv = texcoords[texture_indices[1] - 1],
                .c_uv = texcoords[texture_indices[2] - 1],
                .color = 0xFFFFFFFF};
            array_push(mesh->faces, face);
        }
    }
    array_free(texcoords);
    fclose(file);
}

// // my crappy solution :D when i didnt know sscanf
// void load_obj_file_data_2(char *filename)
// {
//     // Read the contents of the obj file
//     // Load the the vertices and faces in the mesh struct

//     FILE *fptr;
//     fptr = fopen(filename, "r");

//     if (fptr != NULL)
//     {
//         const unsigned MAX_LENGTH = 256;
//         const unsigned int NUMBER_BUFFER_LENGTH = 8; // 0.885739

//         char buffer[MAX_LENGTH];
//         char number_buffer[NUMBER_BUFFER_LENGTH];
//         float vertices[3];
//         int faces[3];

//         int i = 1;
//         int buff_index = 0;

//         while (fgets(buffer, MAX_LENGTH, fptr))
//         {
//             char type = buffer[0];

//             if (buffer[1] == ' ' && (type == 'v' || type == 'f'))
//             {
//                 i = 1;
//                 buff_index = 0;

//                 while (1)
//                 {
//                     if (i == MAX_LENGTH || buffer[i] == '\n')
//                     {
//                         break;
//                     }
//                     if (buffer[i] == ' ')
//                     {
//                         i += 1;

//                         for (int j = 0; j < NUMBER_BUFFER_LENGTH; j++)
//                         {
//                             if (buffer[i + j] == '/')
//                             {
//                                 break;
//                             }
//                             number_buffer[j] = buffer[i + j];
//                         }

//                         if (type == 'v')
//                         {
//                             vertices[buff_index] = atof(number_buffer);
//                         }
//                         else if (type == 'f')
//                         {
//                             faces[buff_index] = atoi(number_buffer);
//                         }

//                         for (int j = 0; j < NUMBER_BUFFER_LENGTH; j++)
//                         {
//                             number_buffer[j] = '\0'; // https://stackoverflow.com/questions/16632765/warning-in-c-assignment-makes-integer-from-pointer-without-a-cast
//                         }
//                         buff_index += 1;
//                     }
//                     i += 1;
//                 }
//                 if (type == 'v')
//                 {
//                     vec3_t vertex_data = {.x = vertices[0], .y = vertices[1], .z = vertices[2]};
//                     array_push(mesh.vertices, vertex_data);
//                 }
//                 else if (type == 'f')
//                 {
//                     face_t face_data = {.a = faces[0], .b = faces[1], .c = faces[2]};
//                     array_push(mesh.faces, face_data);
//                 }
//             }
//         }
//     }
//     else
//     {
//         printf("Not able to read the file");
//     }

//     fclose(fptr);
// }
