#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"
#include "camera.h"
#include "texture.h"
#include "triangle.h"
#include "upng.h"

#define PI 3.14159265359

#define MAX_TRIANGLES_PER_MESH 10000
triangle_t triangles_to_render[MAX_TRIANGLES_PER_MESH];
float delta_time = 0;
int num_triangles_to_render = 0;

mat4_t proj_matrix;
mat4_t view_matrix;
mat4_t world_matrix;

bool is_running = false;
int previous_frame_time = 0;

bool should_cull = false;
void setup(void)
{
    // Allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);
    // Allocate the required memory for the zbuffer
    z_buffer = (float *)malloc(sizeof(float) * window_width * window_height);

    // Creating a SDL texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height);

    // Initialize the perspective projection matrix
    float fov = PI / 3.0; // same as 180 / 3 or 60 deg
    float aspect = (float)window_height / window_width;
    float znear = 0.1;
    float zfar = 100.0;

    proj_matrix = mat4_make_perspective(fov, aspect, znear, zfar);

    // Manually load the hardcoded texture data from the static array
    // mesh_texture = (uint32_t*) REDBRICK_TEXTURE;
    // texture_width = 64;
    // texture_height = 64;

    // Start loading my array of vectors
    // load_cube_mesh_data();
    load_obj_file_data("./assets/f22.obj");
    // loads the texutre info from an external PNG file
    load_png_texture_data("./assets/f22.png");
}

void process_input(void)
{
    SDL_Event event;
    SDL_PollEvent(&event);

    switch (event.type)
    {
    case SDL_QUIT:
        is_running = false;
        break;
    case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_1)
            render_mode = WireframeLine;
        if (event.key.keysym.sym == SDLK_2)
            render_mode = WireframeDot;
        if (event.key.keysym.sym == SDLK_3)
            render_mode = Filled;
        if (event.key.keysym.sym == SDLK_4)
            render_mode = FilledWireframe;
        if (event.key.keysym.sym == SDLK_5)
            render_mode = RenderTextured;
        if (event.key.keysym.sym == SDLK_6)
            render_mode = RenderTexturedWired;
        if (event.key.keysym.sym == SDLK_c)
            should_cull = true;
        if (event.key.keysym.sym == SDLK_d)
            should_cull = false;
        if (event.key.keysym.sym == SDLK_ESCAPE)
            is_running = false;
        break;
    }
}

////////////////////////////////////////////////////////////////////////////////
// Function that receives a 3D vector and returns a projected 2D point
////////////////////////////////////////////////////////////////////////////////
// vec2_t project(vec3_t point)
// {
//     vec2_t projected_point = {
//         .x = (fov_factor * point.x) / point.z,
//         .y = (fov_factor * point.y) / point.z};
//     return projected_point;
// }

void update(void)
{
    // Blocks the main thread so that its a FPS based animation
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }

    delta_time = (SDL_GetTicks() - previous_frame_time) / 1000.0;

    previous_frame_time = SDL_GetTicks();

    // Initialize the counter of triangles to render for the current frame;
    num_triangles_to_render = 0;

    mesh.rotation.x += 0.06 * delta_time;
    mesh.rotation.y += 0.009 * delta_time;
    mesh.rotation.z += 0.02 * delta_time;
    mesh.translation.z = 5.0;
    

    // Change camera position per animation frame
    camera.position.x += 0.0 * delta_time;
    camera.position.y += 0.0 * delta_time;

    // mesh.translation.x += 0.01;

    // Create the view matrix to transform the objects into camera space looking at a hard coded target point
    vec3_t target = { 0, 0, 5.0 };
    vec3_t up = { 0, 1, 0 };
    view_matrix = mat4_look_at(camera.position, target, up);

    // Create a scale, translation and rotation matrix that will be used to multiply the mesh vertices;
    mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh.translation.x, mesh.translation.y, mesh.translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh.rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh.rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh.rotation.z);

    // all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a];
        face_vertices[1] = mesh.vertices[mesh_face.b];
        face_vertices[2] = mesh.vertices[mesh_face.c];

        vec4_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++)
        {
            vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

            world_matrix = mat4_identity();
            // Multiply all matrices and load the world matrix [T]*[R]*[S]*v
            world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_x, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_y, world_matrix);
            world_matrix = mat4_mul_mat4(rotation_matrix_z, world_matrix);
            world_matrix = mat4_mul_mat4(translation_matrix, world_matrix);

            transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

            // Multiply the view matrix with the object vertices to transform everything into camera space
            transformed_vertex = mat4_mul_vec4(view_matrix, transformed_vertex);


            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;
        }

        // Check for backface culling
        vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]); /*   A   */
        vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]); /*  / \  */
        vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]); /* C---B */

        // Get the vectors that go from A to B and A to C to get the cross product
        vec3_t vector_ab = vec3_sub(vector_b, vector_a);
        vec3_t vector_ac = vec3_sub(vector_c, vector_a);
        vec3_normalize(&vector_ab);
        vec3_normalize(&vector_ac);

        // Compute the face normal using the cross product , finding the perpendicular vector - our system is left handed ( z grows inside the monitor )
        // !! Order is important ( if it was a Right Handed coordinate system, it would've been vector_ac X vector_ab )
        vec3_t normal = vec3_cross(vector_ab, vector_ac);

        // normalize the face vector - mutating it
        vec3_normalize(&normal);

        // Find the vector between A Point in the triangle and  the camera origin
        vec3_t origin = {0, 0, 0};
        vec3_t camera_ray = vec3_sub(origin, vector_a);

        // How aligned the camera ray is with the face normal
        float dot_normal_camera = vec3_dot(normal, camera_ray);

        if (should_cull)
        {
            // Bypass the triangles that are looking away from the camera
            if (dot_normal_camera < 0)
            {
                continue;
            }
        }

        vec4_t projected_points[3];

        // Loop all three vertices to perform projection
        for (int j = 0; j < 3; j++)
        {
            projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

            // scale into the view
            projected_points[j].x *= (window_width / 2.0);
            projected_points[j].y *= (window_height / 2.0);

            // Invert the y values to account for the flipped screen y coordinates
            projected_points[j].y *= -1;

            // translate projected points to the middle of the screen
            projected_points[j].x += (window_width / 2);
            projected_points[j].y += (window_height / 2);
        }

        // Calculate the light of the triangle
        float light = -vec3_dot(directional_light.direction, normal);
        uint32_t color = light_apply_intensity(mesh_face.color, light);

        triangle_t projected_triangle = {
            .points = {
                {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w}},
            .color = color,
            .texcoords = {
                {mesh_face.a_uv.u, mesh_face.a_uv.v},
                {mesh_face.b_uv.u, mesh_face.b_uv.v},
                {mesh_face.c_uv.u, mesh_face.c_uv.v},
            },
        };

        // Save the projected triangle in the array of triangles to render
        // triangles_to_render[i] = projected_triangle;
        if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
        {
            triangles_to_render[num_triangles_to_render++] = projected_triangle;
        }
    }
}

void render(void)
{
    draw_grid();

    // draw_filled_triangle(300,100, 50, 400, 500, 700, 0xFFFF00FF);

    // Loop all projected triangles and render them
    for (int i = 0; i < num_triangles_to_render; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangle
        if (render_mode == Filled || render_mode == FilledWireframe)
        {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.color);
        }
        // Draw textured triangle
        if (render_mode == RenderTextured || render_mode == RenderTexturedWired)
        {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                mesh_texture);
        }
        // Draw wireframe
        if (render_mode == WireframeDot || render_mode == WireframeLine || render_mode == FilledWireframe || render_mode == RenderTexturedWired)
        {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFF00);
        }

        // Draw dots
        if (render_mode == WireframeDot)
        {
            // Draw Vertex Points
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
    }

    render_color_buffer();

    clear_color_buffer(0xFF000000);

    clear_z_buffer();

    SDL_RenderPresent(renderer);
}
void free_resources(void)
{
    free(color_buffer);
    free(z_buffer);
    upng_free(png_texture);
    array_free(mesh.faces);
    array_free(mesh.vertices);
}

////////////////////////////////////////////////////////////////////////
// MAIN
////////////////////////////////////////////////////////////////////////
int main(int argc, char *argv[])
{
    is_running = initialize_window();

    setup();

    while (is_running)
    {
        process_input();
        update();
        render();
    }

    destroy_window();
    free_resources();

    return 0;
}
