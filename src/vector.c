#include "vector.h"
#include <math.h>

////////////////////////////////////////////////////////////////////////
// Vec2D Functions
////////////////////////////////////////////////////////////////////////
float vec2_length(vec2_t v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}

////////////////////////////////////////////////////////////////////////
// Vec3D Functions
////////////////////////////////////////////////////////////////////////
float vec3_length(vec3_t v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

vec3_t vec3_rotate_x(vec3_t v, float angle)
{
    vec3_t rotated_vec = {
        .x = v.x,
        .y = v.y * cos(angle) - v.z * sin(angle),
        .z = v.y * sin(angle) + v.z * cos(angle)};
    return rotated_vec;
}
vec3_t vec3_rotate_y(vec3_t v, float angle)
{
    vec3_t rotated_vec = {
        .x = v.x * cos(angle) - v.z * sin(angle),
        .y = v.y,
        .z = v.x * sin(angle) + v.z * cos(angle)};
    return rotated_vec;
}
vec3_t vec3_rotate_z(vec3_t v, float angle)
{
    vec3_t rotated_vec = {
        .x = v.x * cos(angle) - v.y * sin(angle),
        .y = v.x * sin(angle) + v.y * cos(angle),
        .z = v.z};
    return rotated_vec;
}
