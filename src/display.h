#pragma once

#include <SDL2/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include "vector.h"

#define FPS 60
#define FRAME_TARGET_TIME (1000 / FPS)

enum RENDER_MODE_E
{
    WireframeLine,
    WireframeDot,
    Filled,
    FilledWireframe,
    RenderTextured,
    RenderTexturedWired

};

bool initialize_window(void);
void clear_color_buffer(uint32_t color);
void clear_z_buffer(void);
void render_color_buffer(void);
void draw_grid(void);
void draw_rect(int x, int y, int width, int height, uint32_t color);
void draw_pixel(int x, int y, uint32_t color);
void draw_line(int x0, int y0, int x1, int y1, uint32_t color);
uint32_t light_apply_intensity(uint32_t original_color, float percentage_factor);

void destroy_window(void);

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint32_t* color_buffer;
extern float* z_buffer;
extern SDL_Texture* color_buffer_texture;
extern int window_width;
extern int window_height;
extern bool should_cull;
extern enum RENDER_MODE_E render_mode;