#include "texture.h"
#include <stdio.h>



tex2_t tex2_clone(tex2_t* tex) {
    tex2_t result = {
        .u = tex->u,
        .v = tex->v
    };
    return result;
}
