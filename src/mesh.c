#include "mesh.h"
#include "array.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

mesh_t mesh = {
    .vertices = NULL,
    .faces = NULL,
    .rotation = {0, 0, 0},
    .scale = {1.0, 1.0, 1.0},
    .translation = {0, 0, 0}
};

vec3_t cube_vertices[N_CUBE_VERTICES] = {
    { .x = -1, .y = -1, .z = -1 }, // 1
    { .x = -1, .y =  1, .z = -1 }, // 2
    { .x =  1, .y =  1, .z = -1 }, // 3
    { .x =  1, .y = -1, .z = -1 }, // 4
    { .x =  1, .y =  1, .z =  1 }, // 5
    { .x =  1, .y = -1, .z =  1 }, // 6
    { .x = -1, .y =  1, .z =  1 }, // 7
    { .x = -1, .y = -1, .z =  1 }  // 8
};

face_t cube_faces[N_CUBE_FACES] = {
    // front
    { .a = 1, .b = 2, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 1, .b = 3, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // right
    { .a = 4, .b = 3, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 4, .b = 5, .c = 6, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // back
    { .a = 6, .b = 5, .c = 7, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 7, .c = 8, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // left
    { .a = 8, .b = 7, .c = 2, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 8, .b = 2, .c = 1, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // top
    { .a = 2, .b = 7, .c = 5, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 2, .b = 5, .c = 3, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF },
    // bottom
    { .a = 6, .b = 8, .c = 1, .a_uv = { 0, 0 }, .b_uv = { 0, 1 }, .c_uv = { 1, 1 }, .color = 0xFFFFFFFF },
    { .a = 6, .b = 1, .c = 4, .a_uv = { 0, 0 }, .b_uv = { 1, 1 }, .c_uv = { 1, 0 }, .color = 0xFFFFFFFF }
};
void load_cube_mesh_data(void)
{
    for (int i = 0; i < N_CUBE_VERTICES; i++)
    {
        vec3_t cube_vertex = cube_vertices[i];
        array_push(mesh.vertices, cube_vertex);
    }
    for (int i = 0; i < N_CUBE_FACES; i++)
    {
        face_t cube_face = cube_faces[i];
        array_push(mesh.faces, cube_face);
    }
}

void load_obj_file_data(char *filename)
{
    FILE *file;
    file = fopen(filename, "r");

    char line[1024];

    while (fgets(line, 1024, file))
    {
        // Vertex information
        if (strncmp(line, "v ", 2) == 0)
        {
            vec3_t vertex;
            sscanf(line, "v %f %f %f", &vertex.x, &vertex.y, &vertex.z);
            array_push(mesh.vertices, vertex);
        }
        if (strncmp(line, "f ", 2) == 0)
        {
            int vertex_indices[3];
            int texture_indices[3];
            int normal_indices[3];
            sscanf(line, "f %d/%d/%d %d/%d/%d %d/%d/%d", &vertex_indices[0], &texture_indices[0], &normal_indices[0], &vertex_indices[1], &texture_indices[1], &normal_indices[1], &vertex_indices[2], &texture_indices[2], &normal_indices[2]);

            face_t face = {
                .a = vertex_indices[0],
                .b = vertex_indices[1],
                .c = vertex_indices[2],
                .color = 0xFFFFFFFF
                };

            array_push(mesh.faces, face);
        }
    }
}

// my crappy solution :D
void load_obj_file_data_2(char *filename)
{
    // Read the contents of the obj file
    // Load the the vertices and faces in the mesh struct

    FILE *fptr;
    fptr = fopen(filename, "r");

    if (fptr != NULL)
    {
        const unsigned MAX_LENGTH = 256;
        const unsigned int NUMBER_BUFFER_LENGTH = 8; // 0.885739

        char buffer[MAX_LENGTH];
        char number_buffer[NUMBER_BUFFER_LENGTH];
        float vertices[3];
        int faces[3];

        int i = 1;
        int buff_index = 0;

        while (fgets(buffer, MAX_LENGTH, fptr))
        {
            char type = buffer[0];

            if (buffer[1] == ' ' && (type == 'v' || type == 'f'))
            {
                i = 1;
                buff_index = 0;

                while (1)
                {
                    if (i == MAX_LENGTH || buffer[i] == '\n')
                    {
                        break;
                    }
                    if (buffer[i] == ' ')
                    {
                        i += 1;

                        for (int j = 0; j < NUMBER_BUFFER_LENGTH; j++)
                        {
                            if (buffer[i + j] == '/')
                            {
                                break;
                            }
                            number_buffer[j] = buffer[i + j];
                        }

                        if (type == 'v')
                        {
                            vertices[buff_index] = atof(number_buffer);
                        }
                        else if (type == 'f')
                        {
                            faces[buff_index] = atoi(number_buffer);
                        }

                        for (int j = 0; j < NUMBER_BUFFER_LENGTH; j++)
                        {
                            number_buffer[j] = '\0'; // https://stackoverflow.com/questions/16632765/warning-in-c-assignment-makes-integer-from-pointer-without-a-cast
                        }
                        buff_index += 1;
                    }
                    i += 1;
                }
                if (type == 'v')
                {
                    vec3_t vertex_data = {.x = vertices[0], .y = vertices[1], .z = vertices[2]};
                    array_push(mesh.vertices, vertex_data);
                }
                else if (type == 'f')
                {
                    face_t face_data = {.a = faces[0], .b = faces[1], .c = faces[2]};
                    array_push(mesh.faces, face_data);
                }
            }
        }
    }
    else
    {
        printf("Not able to read the file");
    }

    fclose(fptr);
}
