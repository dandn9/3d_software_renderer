#include "display.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

static uint32_t *color_buffer = NULL;
static float *z_buffer = NULL;

static SDL_Texture *color_buffer_texture = NULL;
static int window_width;
static int window_height;

static enum RENDER_MODE_E render_mode;

int get_window_width(void)
{
    return window_width;
}
int get_window_height(void)
{
    return window_width;
}
void set_render_method(int method)
{
    render_mode = method;
}
bool should_render_wireframe(void)
{
    return render_mode == WireframeDot || render_mode == WireframeLine || render_mode == FilledWireframe || render_mode == RenderTexturedWired;
}
bool should_render_dots(void)
{
    return render_mode == WireframeDot;
}
bool should_render_textured_triangle(void)
{
    return render_mode == RenderTextured || render_mode == RenderTexturedWired;
}
bool should_render_filled_triangle(void)
{
    return (render_mode == Filled || render_mode == FilledWireframe);
}

float get_zbuffer_at(int x, int y)
{
    if (x < 0 || x >= window_width || y < 0 || y >= window_height)
    {
        return 1.0;
    }
    return z_buffer[(y * window_width) + x];
}
void update_zbuffer_at(int x, int y, float val)
{
    if (x < 0 || x >= window_width || y < 0 || y >= window_height)
    {
        return;
    }
    z_buffer[(y * window_width) + x] = val;
}
bool initialize_window(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
    {

        fprintf(stderr, "Error initializing SDL \n");
        return false;
    }

    // Use SDL to query what is the screen's w & h
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(0, &display_mode);
    int fullscreen_width = display_mode.w;
    int fullscreen_height = display_mode.h;

    window_width = fullscreen_width / 5;
    window_height = fullscreen_height / 5;

    // Create a SDL Window
    window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        fullscreen_width,
        fullscreen_height,
        SDL_WINDOW_BORDERLESS);

    if (!window)
    {

        fprintf(stderr, "Error creating SDL window");
        return false;
    }
    //  Create a SDL Renderer
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer)
    {

        fprintf(stderr, "Error creating SDL renderer");
        return false;
    }

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
    return true;
}

void draw_pixel(int x, int y, uint32_t color)
{
    // better with or than and because of short circuit!
    if (x < 0 || x >= window_width || y < 0 || y >= window_height)
    {
        return;
    }
    color_buffer[(window_width * y) + x] = color;
}

void draw_rect(int x, int y, int width, int height, uint32_t color)
{
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            int current_x = x + i;
            int current_y = y + j;
            draw_pixel(current_x, current_y, color);
        }
    }
}

void draw_grid(void)
{
    // Draw a background that fills the entire window
    // lines should be rendererd at every row/col multiple by 10

    for (int y = 0; y < window_height; y += 10)
    {
        for (int x = 0; x < window_width; x += 10)
        {
            if (y % 10 == 0 || x % 10 == 0)
            {
                color_buffer[window_width * y + x] = 0xFF333333;
            }
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, uint32_t color)
{
    int delta_x = (x1 - x0);
    int delta_y = (y1 - y0);

    int side_length = abs(delta_x) >= abs(delta_y) ? abs(delta_x) : abs(delta_y);

    // amount to increment x or y
    float inc_x = delta_x / (float)side_length;
    float inc_y = delta_y / (float)side_length;

    float current_x = x0;
    float current_y = y0;
    for (int i = 0; i <= side_length; i++)
    {
        draw_pixel(round(current_x), round(current_y), color);
        current_x += inc_x;
        current_y += inc_y;
    }
}

void render_color_buffer(void)
{

    SDL_UpdateTexture(
        color_buffer_texture,
        NULL,
        color_buffer,
        (int)window_width * sizeof(uint32_t));

    SDL_RenderCopy(
        renderer,
        color_buffer_texture,
        NULL,
        NULL);

    SDL_RenderPresent(renderer);
}

void clear_color_buffer(uint32_t color)
{
    for (int i = 0; i < window_width * window_height; i++)
    {
        color_buffer[i] = color;
    }
}

void clear_z_buffer(void)
{
    for (int i = 0; i < window_width * window_height; i++)
    {
        z_buffer[i] = 1.0;
    }
}

void destroy_window(void)
{

    free(color_buffer);
    free(z_buffer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
