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
#include "clipping.h"

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

bool should_cull = true;
void setup(void)
{
    // Allocate the required memory in bytes to hold the color buffer

    set_render_method(RenderTextured);

    init_light(vec3_new(0, 0, 1));
    // Initialize the perspective projection matrix
    float aspecty = (float)get_window_height() / get_window_width();
    float aspectx = (float)get_window_width() / get_window_height();
    float fovy = PI / 3.0;                            // same as 180 / 3 or 60 deg
    float fovx = 2.0 * atan(tan(fovy / 2) * aspectx); // same as 180 / 3 or 60 deg

    float z_near = 0.1;
    float z_far = 100.0;

    proj_matrix = mat4_make_perspective(fovy, aspecty, z_near, z_far);

    // initialize the frustum planes with a point and a normal
    init_frustum_planes(fovx, fovy, z_near, z_far);

    // Manually load the hardcoded texture data from the static array
    // mesh_texture = (uint32_t*) REDBRICK_TEXTURE;
    load_mesh("./assets/f22.obj", "./assets/f22.png", vec3_new(1, 1, 1), vec3_new(+3, 0, 8), vec3_new(0, 0, 0));
    load_mesh("./assets/cube.obj", "./assets/cube.png", vec3_new(1, 1, 1), vec3_new(-3, 0, 8), vec3_new(0, 0, 0));
}

void process_input(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_QUIT:
            is_running = false;
            break;
        case SDL_KEYDOWN:
            if (event.key.keysym.sym == SDLK_1)
            {

                set_render_method(WireframeLine);
                break;
            }
            if (event.key.keysym.sym == SDLK_2)
            {

                set_render_method(WireframeDot);
                break;
            }
            if (event.key.keysym.sym == SDLK_3)
            {

                set_render_method(Filled);
                break;
            }
            if (event.key.keysym.sym == SDLK_4)
            {

                set_render_method(FilledWireframe);
                break;
            }
            if (event.key.keysym.sym == SDLK_5)
            {

                set_render_method(RenderTextured);
                break;
            }
            if (event.key.keysym.sym == SDLK_6)
            {

                set_render_method(RenderTexturedWired);
                break;
            }
            if (event.key.keysym.sym == SDLK_c)
            {

                should_cull = true;
                break;
            }
            if (event.key.keysym.sym == SDLK_v)
            {

                should_cull = false;
                break;
            }
            if (event.key.keysym.sym == SDLK_ESCAPE)
            {
                is_running = false;
                break;
            }
            if (event.key.keysym.sym == SDLK_w)
            {
                rotate_camera_pitch(+3.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_s)
            {
                rotate_camera_pitch(-3.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_RIGHT)
            {
                rotate_camera_yaw(+1.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_LEFT)
            {
                rotate_camera_yaw(-1.0 * delta_time);
                break;
            }
            if (event.key.keysym.sym == SDLK_UP)
            {
                update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                update_camera_position(vec3_add(get_camera_position(), get_camera_forward_velocity()));
                break;
            }
            if (event.key.keysym.sym == SDLK_DOWN)
            {
                update_camera_forward_velocity(vec3_mul(get_camera_direction(), 5.0 * delta_time));
                update_camera_position(vec3_sub(get_camera_position(), get_camera_forward_velocity()));
                break;
            }
            break;
        }
    }
}

// model space -> world space -> camera space -> clipping -> projection -> image space -> screen space
void process_graphics_pipeline_stages(mesh_t *mesh)
{
    // Create a scale, translation and rotation matrix that will be used to multiply the mesh vertices;
    mat4_t scale_matrix = mat4_make_scale(mesh->scale.x, mesh->scale.y, mesh->scale.z);
    mat4_t translation_matrix = mat4_make_translation(mesh->translation.x, mesh->translation.y, mesh->translation.z);
    mat4_t rotation_matrix_x = mat4_make_rotation_x(mesh->rotation.x);
    mat4_t rotation_matrix_y = mat4_make_rotation_y(mesh->rotation.y);
    mat4_t rotation_matrix_z = mat4_make_rotation_z(mesh->rotation.z);

    vec3_t target = get_camera_lookat_target();
    vec3_t up_direction = vec3_new(0, 1, 0);
    // Create the view matrix to transform the objects into camera space looking at a hard coded target point
    view_matrix = mat4_look_at(get_camera_position(), target, up_direction);

    // all triangle faces of our mesh
    int num_faces = array_length(mesh->faces);
    for (int i = 0; i < num_faces; i++)
    {
        face_t mesh_face = mesh->faces[i];

        vec3_t face_vertices[3];
        face_vertices[0] = mesh->vertices[mesh_face.a];
        face_vertices[1] = mesh->vertices[mesh_face.b];
        face_vertices[2] = mesh->vertices[mesh_face.c];

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
        // Calculate the triangle face normal
        vec3_t face_normal = get_triangle_normal(transformed_vertices);

        if (should_cull)
        {

            // Find the vector between A Point in the triangle and  the camera origin
            vec3_t camera_ray = vec3_sub(vec3_new(0, 0, 0), vec3_from_vec4(transformed_vertices[0]));

            // How aligned the camera ray is with the face normal
            float dot_normal_camera = vec3_dot(face_normal, camera_ray);
            // Bypass the triangles that are looking away from the camera
            if (dot_normal_camera < 0)
            {
                continue;
            }
        }
        // Create  a polygon from the original transform create_polygon
        polygon_t polygon = create_polygon_from_triangle(
            vec3_from_vec4(transformed_vertices[0]),
            vec3_from_vec4(transformed_vertices[1]),
            vec3_from_vec4(transformed_vertices[2]),
            mesh_face.a_uv,
            mesh_face.b_uv,
            mesh_face.c_uv);

        // Clip the polygon and return a new polygon with potential new vertices
        clip_polygon(&polygon);

        triangle_t triangles_after_clipping[MAX_NUM_POLY_TRIANGLES];
        int num_triangles_after_clipping = 0;
        triangles_from_polygon(&polygon, triangles_after_clipping, &num_triangles_after_clipping);

        for (int t = 0; t < num_triangles_after_clipping; t++)
        {
            triangle_t triangle_after_clipping = triangles_after_clipping[t];

            vec4_t projected_points[3];

            // Loop all three vertices to perform projection
            for (int j = 0; j < 3; j++)
            {
                projected_points[j] = mat4_mul_vec4_project(proj_matrix, triangle_after_clipping.points[j]);

                // scale into the view
                projected_points[j].x *= (get_window_width() / 2.0);
                projected_points[j].y *= (get_window_height() / 2.0);

                // Invert the y values to account for the flipped screen y coordinates
                projected_points[j].y *= -1;

                // translate projected points to the middle of the screen
                projected_points[j].x += (get_window_width() / 2);
                projected_points[j].y += (get_window_height() / 2);
            }

            // Calculate the light of the triangle
            float light = -vec3_dot(get_light_direction(), face_normal);
            uint32_t color = light_apply_intensity(mesh_face.color, light);

            triangle_t triangle_to_render = {
                .points = {
                    {projected_points[0].x, projected_points[0].y, projected_points[0].z, projected_points[0].w},
                    {projected_points[1].x, projected_points[1].y, projected_points[1].z, projected_points[1].w},
                    {projected_points[2].x, projected_points[2].y, projected_points[2].z, projected_points[2].w}},
                .color = color,
                .texcoords = {
                    {triangle_after_clipping.texcoords[0].u, triangle_after_clipping.texcoords[0].v},
                    {triangle_after_clipping.texcoords[1].u, triangle_after_clipping.texcoords[1].v},
                    {triangle_after_clipping.texcoords[2].u, triangle_after_clipping.texcoords[2].v},
                },
                .texture = mesh->texture};

            // Save the projected triangle in the array of triangles to render
            // triangles_to_render[i] = projected_triangle;
            if (num_triangles_to_render < MAX_TRIANGLES_PER_MESH)
            {
                triangles_to_render[num_triangles_to_render++] = triangle_to_render;
            }
        }
    }
}

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

    // mesh.rotation.x += 0.00 * delta_time;
    // mesh.rotation.y += 0.200 * delta_time;
    // mesh.rotation.z += 0.00 * delta_time;
    // mesh.translation.z = 5.0;

    // mesh.translation.x += 0.01;

    for (int mesh_index = 0; mesh_index < get_num_meshes(); mesh_index++)
    {
        mesh_t *mesh = get_mesh(mesh_index);
        process_graphics_pipeline_stages(mesh);
    }
}

void render(void)
{
    clear_color_buffer(0xFF000000);
    clear_z_buffer();
    draw_grid();

    // draw_filled_triangle(300,100, 50, 400, 500, 700, 0xFFFF00FF);

    // Loop all projected triangles and render them
    for (int i = 0; i < num_triangles_to_render; i++)
    {
        triangle_t triangle = triangles_to_render[i];

        // Draw filled triangle
        if (should_render_filled_triangle())
        {
            draw_filled_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w,
                triangle.color);
        }
        // Draw textured triangle
        if (should_render_textured_triangle())
        {
            draw_textured_triangle(
                triangle.points[0].x, triangle.points[0].y, triangle.points[0].z, triangle.points[0].w, triangle.texcoords[0].u, triangle.texcoords[0].v,
                triangle.points[1].x, triangle.points[1].y, triangle.points[1].z, triangle.points[1].w, triangle.texcoords[1].u, triangle.texcoords[1].v,
                triangle.points[2].x, triangle.points[2].y, triangle.points[2].z, triangle.points[2].w, triangle.texcoords[2].u, triangle.texcoords[2].v,
                triangle.texture);
        }
        // Draw wireframe
        if (should_render_wireframe())
        {
            draw_triangle(
                triangle.points[0].x, triangle.points[0].y,
                triangle.points[1].x, triangle.points[1].y,
                triangle.points[2].x, triangle.points[2].y,
                0xFFFFFF00);
        }

        // Draw dots
        if (should_render_dots())
        {
            // Draw Vertex Points
            draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF0000);
            draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF0000);
        }
    }

    render_color_buffer();
}
void free_resources(void)
{
    free_meshes();
    destroy_window();
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

    free_resources();

    return 0;
}
