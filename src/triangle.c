#include "triangle.h"
#include "display.h"


void int_swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void fill_flat_top_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {

}
// Draw a filled triangle with a flat bottom
//     (x0, y0)
//        /\
//       /  \                                
//      /    \                               
//     / ____ \                              
// (x1, y1)    (x2, y2)                                  
//                                         

void fill_flat_bottom_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color) {
    // find the two slopes (two triangle legs)
    float inv_slope_1 = (float)(x1 - x0)/(y1 - y0);
    float inv_slope_2 = (float)(x2 - x0)/(y2 - y0);

    float x_start = x0;
    float x_end = x0;
    // Loop all the scalines from top to bottom
    for(int y = y0; y <=  y2; y++){
        draw_line(x_start, y, x_end, y, color);
        x_start += inv_slope_1;
        x_end += inv_slope_2;

    }
}

void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color){
    // We need to sort out the indices by y-coordinate
    if(y0 > y1){
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }
    if(y1 > y2) {
        int_swap(&y1, &y2);
        int_swap(&x1, &x2);
    }
    if(y0 > y1){
        int_swap(&y0, &y1);
        int_swap(&x0, &x1);
    }

    // Calculate the (Mx My) vertex using triangle similarity [look notes]
    int My = y1;
    int Mx = ((float)((x2 - x0) * (y1 - y0))/(float)(y2 - y0)) + x0;

    // Draw flat bottom triangle
    fill_flat_bottom_triangle(x0, y0, x1, y1, Mx, My, color);
    // Draw flat top triangle
    fill_flat_top_triangle(x1, y1, Mx, My, x2, y2, color);
}