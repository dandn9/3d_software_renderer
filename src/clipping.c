#include "clipping.h"
#include <math.h>

#define NUM_PLANES 6
plane_t frustum_planes[NUM_PLANES];

///////////////////////////////////////////////////////////////////////////////
// Frustum planes are defined by a point and a normal vector
///////////////////////////////////////////////////////////////////////////////
// Near plane   :  P=(0, 0, znear), N=(0, 0,  1)
// Far plane    :  P=(0, 0, zfar),  N=(0, 0, -1)
// Top plane    :  P=(0, 0, 0),     N=(0, -cos(fov/2), sin(fov/2))
// Bottom plane :  P=(0, 0, 0),     N=(0, cos(fov/2), sin(fov/2))
// Left plane   :  P=(0, 0, 0),     N=(cos(fov/2), 0, sin(fov/2))
// Right plane  :  P=(0, 0, 0),     N=(-cos(fov/2), 0, sin(fov/2))
///////////////////////////////////////////////////////////////////////////////
//
//           /|\
//         /  | | 
//       /\   | |
//     /      | |
//  P*|-->  <-|*|   ----> +z-axis
//     \      | |
//       \/   | |
//         \  | | 
//           \|/
//
///////////////////////////////////////////////////////////////////////////////
void init_frustum_planes(float fovx, float fovy, float z_near, float z_far) {
    // Explanation: https://courses.pikuma.com/courses/take/learn-computer-graphics-programming/lessons/15960322-initializing-an-array-of-frustum-planes
	float cos_half_fovx = cos(fovx / 2);
	float sin_half_fovx = sin(fovx / 2);
	float cos_half_fovy = cos(fovy / 2);
	float sin_half_fovy = sin(fovy / 2);

    vec3_t origin = { 0, 0, 0 };

	frustum_planes[LEFT_FRUSTUM_PLANE].point = origin;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.x = cos_half_fovx;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[LEFT_FRUSTUM_PLANE].normal.z = sin_half_fovx;

	frustum_planes[RIGHT_FRUSTUM_PLANE].point = origin;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.x = -cos_half_fovx;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[RIGHT_FRUSTUM_PLANE].normal.z = sin_half_fovx;

	frustum_planes[TOP_FRUSTUM_PLANE].point = origin;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.y = -cos_half_fovy;
	frustum_planes[TOP_FRUSTUM_PLANE].normal.z = sin_half_fovy;

	frustum_planes[BOTTOM_FRUSTUM_PLANE].point = origin;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.y = cos_half_fovy;
	frustum_planes[BOTTOM_FRUSTUM_PLANE].normal.z = sin_half_fovy;

	frustum_planes[NEAR_FRUSTUM_PLANE].point = (vec3_t){0, 0, z_near};
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[NEAR_FRUSTUM_PLANE].normal.z = 1;

    frustum_planes[FAR_FRUSTUM_PLANE].point = (vec3_t){0, 0, z_far};
	frustum_planes[FAR_FRUSTUM_PLANE].normal.x = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.y = 0;
	frustum_planes[FAR_FRUSTUM_PLANE].normal.z = -1;
}

polygon_t create_polygon_from_triangle(vec3_t v0, vec3_t v1, vec3_t v2) {
	polygon_t polygon = {
		.vertices = { v0, v1, v2 },
		.num_vertices = 3
	};
	return polygon;
}

void triangles_from_polygon(polygon_t* polygon, triangle_t triangles[], int* num_triangles) {
	for(int i = 0; i < polygon->num_vertices - 2; i++) {
		int index0 = 0;
		int index1 = i + 1;
		int index2 = i + 2;

		triangles[i].points[0] = vec4_from_vec3(polygon->vertices[index0]);
		triangles[i].points[1] = vec4_from_vec3(polygon->vertices[index1]);
		triangles[i].points[2] = vec4_from_vec3(polygon->vertices[index2]);
	}
	*num_triangles = polygon->num_vertices - 2;
}
void clip_polygon_against_plane(polygon_t* polygon, int plane){
	vec3_t plane_point = frustum_planes[plane].point;
	vec3_t plane_normal = frustum_planes[plane].normal;

	// the arrayof inside vertices that will be part of the final polygon returned via out parameter(polygon)
	vec3_t inside_vertices[MAX_NUM_POLY_VERTICES];
	int num_inside_vertices = 0;

	vec3_t* current_vertex = &polygon->vertices[0];
	vec3_t* previous_vertex = &polygon->vertices[polygon->num_vertices - 1];

	// Dotq1 = dot(planeN, q1-p)
	float current_dot = 0;
	float previous_dot = vec3_dot(vec3_sub(*previous_vertex, plane_point), plane_normal);

	while(current_vertex != &polygon->vertices[polygon->num_vertices]) {
		current_dot = vec3_dot(vec3_sub(*current_vertex, plane_point), plane_normal);
		// if we changed from inside to outside or vice-versa
		if (current_dot * previous_dot < 0) {
			// t = dotq1 / (dotq1 - dotq2)
			float t = previous_dot / (previous_dot - current_dot);
			// intersection point I = q1 + t(q2-q1)
			vec3_t intersection_point = vec3_clone(current_vertex);
			intersection_point = vec3_sub(intersection_point, *previous_vertex);
			intersection_point = vec3_mul(intersection_point, t);
			intersection_point = vec3_add(intersection_point, *previous_vertex);
			inside_vertices[num_inside_vertices] = vec3_clone(&intersection_point);	
			num_inside_vertices++;
		}

		if(current_dot > 0) {
			inside_vertices[num_inside_vertices] = vec3_clone(current_vertex);
			num_inside_vertices++;
		}
		// Move to the next vertex
		previous_dot = current_dot;
		previous_vertex = current_vertex;
		current_vertex++;
	}
	for (int i = 0; i < num_inside_vertices; i++) {
		polygon->vertices[i] = vec3_clone(&inside_vertices[i]);
	}
	polygon->num_vertices = num_inside_vertices;
}

void clip_polygon(polygon_t* polygon) {
	clip_polygon_against_plane(polygon, LEFT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, RIGHT_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, TOP_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, BOTTOM_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, NEAR_FRUSTUM_PLANE);
	clip_polygon_against_plane(polygon, FAR_FRUSTUM_PLANE);
}