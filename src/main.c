#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>
#include "display.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

triangle_t *triangles_to_render = NULL;

vec3_t camera_position = {.x = 0, .y = 0, .z = 0};

float fov_factor = 640;

bool is_running = false;
int previous_frame_time = 0;

bool should_cull = true;
enum RENDER_MODE { 
    WireframeLine,
    WireframeDot,
    Filled,
    FilledWireframe

} render_mode;


void setup(void)
{
    // Allocate the required memory in bytes to hold the color buffer
    color_buffer = (uint32_t *)malloc(sizeof(uint32_t) * window_width * window_height);

    // Creating a SDL texture that is used to display the color buffer
    color_buffer_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        window_width,
        window_height);

    // Start loading my array of vectors
    load_cube_mesh_data();
    // load_obj_file_data("./assets/cube.obj");
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
vec2_t project(vec3_t point)
{
    vec2_t projected_point = {
        .x = (fov_factor * point.x) / point.z,
        .y = (fov_factor * point.y) / point.z};
    return projected_point;
}

void update(void)
{
    // Blocks the main thread so that its a FPS based animation
    int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

    if (time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME)
    {
        SDL_Delay(time_to_wait);
    }

    previous_frame_time = SDL_GetTicks();

    // Initialize the array of triangles to render;
    triangles_to_render = NULL;

    mesh.rotation.x += 0.01;
    mesh.rotation.y += 0.01;
    mesh.rotation.z += 0.01;

    // all triangle faces of our mesh
    int num_faces = array_length(mesh.faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh.faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh.vertices[mesh_face.a - 1];
        face_vertices[1] = mesh.vertices[mesh_face.b - 1];
        face_vertices[2] = mesh.vertices[mesh_face.c - 1];

        vec3_t transformed_vertices[3];

        // Loop all three vertices of this current face and apply transformations
        for (int j = 0; j < 3; j++)
        {
            vec3_t transformed_vertex = face_vertices[j];

            transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
            transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
            transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

            // Translate the vertex away from camera
            transformed_vertex.z += -5;

            // Save transformed vertex in the array of transformed vertices
            transformed_vertices[j] = transformed_vertex;

        }
        if(should_cull){
            // Check for backface culling
            vec3_t vector_a = transformed_vertices[0]; /*   A   */
            vec3_t vector_b = transformed_vertices[1]; /*  / \  */
            vec3_t vector_c = transformed_vertices[2]; /* C---B */

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

            // Find the vector between a Point in the triangle and  the camera origin
            vec3_t camera_ray = vec3_sub(camera_position, vector_a);

            // How aligned the camera ray is with the face normal
            float dot_normal_camera = vec3_dot(normal, camera_ray);

            // Bypass the triangles that are looking away from the camera
            if(dot_normal_camera < 0){
                continue;
            }
        }


        vec2_t projected_points[3];
        // Loop all three vertices to perform projection
        for (int j = 0; j < 3; j++) {
            projected_points[j] = project(transformed_vertices[j]);

            // Scale and translate projected points to the middle of the screen
            projected_points[j].x += (window_width / 2);
            projected_points[j].y += (window_height / 2);

        }

        triangle_t projected_triangle =  {
            .points = {
                {projected_points[0].x, projected_points[0].y},
                {projected_points[1].x, projected_points[1].y},
                {projected_points[2].x, projected_points[2].y}
                 },
            .color = mesh_face.color
        };


        // Save the projected triangle in the array of triangles to render
        // triangles_to_render[i] = projected_triangle;
        array_push(triangles_to_render, projected_triangle);


        
    }
}

void render(void)
{
    draw_grid();


    // draw_filled_triangle(300,100, 50, 400, 500, 700, 0xFFFF00FF);
    

    // Loop all projected triangles and render them
    int num_triangles = array_length(triangles_to_render);
    for (int i = 0; i < num_triangles; i++)
    {
        triangle_t triangle = triangles_to_render[i];


        if(render_mode == Filled || render_mode == FilledWireframe){
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                triangle.color);
        }
        if(render_mode == WireframeDot || render_mode == WireframeLine || render_mode == FilledWireframe){
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFF00);

        }

        if(render_mode == WireframeDot){
            // Draw Vertex Points
            draw_rect(triangle.points[0].x -3, triangle.points[0].y -3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x -3, triangle.points[1].y -3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x -3, triangle.points[2].y -3, 6, 6, 0xFFFF0000);

        }


    }

    // clear the array of triangles to render
    array_free(triangles_to_render);

    render_color_buffer();

    clear_color_buffer(0xFF000000);

    SDL_RenderPresent(renderer);
}
void free_resources(void)
{
    free(color_buffer);
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
