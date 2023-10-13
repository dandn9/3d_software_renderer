#include "texture.h"
#include <stdio.h>

int texture_width = 64;
int texture_height = 64;
uint32_t* mesh_texture = NULL;
upng_t* png_texture = NULL;

void load_png_texture_data(char* filename) {
    png_texture = upng_new_from_file(filename);
    if (png_texture != NULL) {
        upng_decode(png_texture);
        if (upng_get_error(png_texture) == UPNG_EOK) {
            mesh_texture = (uint32_t*)upng_get_buffer(png_texture);
            texture_width = upng_get_width(png_texture);
            texture_height = upng_get_height(png_texture);
        }
    }
}

tex2_t tex2_clone(tex2_t* tex) {
    tex2_t result = {
        .u = tex->u,
        .v = tex->v
    };
    return result;
}
