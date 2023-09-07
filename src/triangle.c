#include "triangle.h"
#include "swap.h"
#include "display.h"

/* Draw a filled triangle with a flat top, by starting from the lowest point
//          (x0,y0)------(x1,y1)
//                \       /
//                 \     /
//                  \  /
//                   (x2, y2)
*/
void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{

    float inv_slope_1 = (float)(x2 - x0) / (y2 - y0);
    float inv_slope_2 = (float)(x2 - x1) / (y2 - y1);

    float x_start = x2;
    float x_end = x2;

    // Loop all the scalines from top to bottom
    for (int y = y2; y >= y0; y--)
    {
        draw_line(x_start, y, x_end, y, color);
        x_start -= inv_slope_1;
        x_end -= inv_slope_2;
    }
}
/* Draw a filled triangle with a flat bottom
//     (x0, y0)
//        /
//       /  \
//      /    \
//     / ____ \
// (x1, y1)    (x2, y2)
//
*/

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x1 - x0) / (y1 - y0);
    float inv_slope_2 = (float)(x2 - x0) / (y2 - y0);

    float x_start = x0;
    float x_end = x0;
    // Loop all the scalines from top to bottom
    for (int y = y0; y <= y2; y++)
    {
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;
    }
}
void draw_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    draw_line(x0, y0, x1, y1, color);
    draw_line(x1, y1, x2, y2, color);
    draw_line(x2, y2, x0, y0, color);
}
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color)
{
    // We need to sort out the indices by y-coordinate
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    if (y1 == y2)
    {
        // if the y1 is equal to y2, it means we already have a flat bottom, so we dont need to render the flat top triangle ~ this also avoids division by 0
        fill_flat_bottom_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else if (y0 == y1)
    {
        // it means we have a flat top
        fill_flat_top_triangle(x0, y0, x1, y1, x2, y2, color);
    }
    else
    {
        // Calculate the (Mx My) vertex using triangle similarity [look notes]
        int My = y1;
        int Mx = ((float)((x2 - x0) * (y1 - y0)) / (float)(y2 - y0)) + x0;
        // Draw flat bottom triangle
        fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
        // Draw flat top triangle
        fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
    }
}

// Draw a textured triangle with a flat-top/flat bottom method
void draw_textured_triangle(
    int x0, int y0, float u0, float v0,
    int x1, int y1, float u1, float v1,
    int x2, int y2, float u2, float v2,
    uint32_t *texture)
{

    // We need to sort the vertices by their y coordinate ascending (y0 < y1 < y2)
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }
    if (y1 > y2)
    {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
        float_swap(&u1, &u2);
        float_swap(&v1, &v2);
    }
    if (y0 > y1)
    {
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
        float_swap(&u0, &u1);
        float_swap(&v0, &v1);
    }

    // Render the upper part of the triangle (flat-bottom)
    float inv_slope_1 = 0;
    float inv_slope_2 = 0;

    if (y1 - y0 != 0)
        inv_slope_1 = (float)(x1 - x0) / abs(y1 - y0);
    if (y2 - y0 != 0)
        inv_slope_2 = (float)(x2 - x0) / abs(y2 - y0);
    printf("\nINVERSE SLOPE %f %f", inv_slope_1, inv_slope_2);

    if (y1 - y0 != 0)
    {
        for (int y = y0; y < y1; y++)
        {
            int x_start = x1 + (y - y1) * inv_slope_1;
            int x_end = x0 + (y - y0) * inv_slope_2;

            if (x_end < x_start)
            {
                int_swap(&x_start, &x_end);
            }

            for (int x = x_start; x < x_end; x++)
            {
                // Draw our pixel with the color that comes from the texture
                draw_pixel(x, y, 0xFFFF00FF);
            }
        }
    }

    // Render the bottom part of the triangle (flat-top)
}