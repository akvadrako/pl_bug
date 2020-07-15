
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <wayland-client.h>

#include "gl/common.h"
#include "gl/color.h"
#include "gl/quad.h"
#include "gl/loadpng.h"

#include "wayeasy/egl.h"
#include "wayeasy/shell.h"

int width = 200;
int height = 200;

int main(int argc, char **argv)
{
    egl_app_begin();
    quad_init();
    color_engine_init();
    
    GLuint srctex = loadpng_texture("Lenna.png");
    GLuint nocstex = gl_create_2d_texture(GL_RGBA8, width, height);
    GLuint cstex = gl_create_2d_texture(GL_RGBA8, width, height);
    
    struct color_texture src = {
        .texture_id = srctex,
        .color_depth = 8,
        .sample_depth = 8,
        .width = width,
        .height = height,
        .color_space = pl_color_space_srgb,
    };

    struct color_texture nocs = {
        .texture_id = nocstex,
        .color_depth = 8,
        .sample_depth = 8,
        .width = width,
        .height = height,
        .icc_profile = load_icc_profile("invert-22.icc")
    };
    
    struct color_texture cs = {
        .texture_id = cstex,
        .color_depth = 8,
        .sample_depth = 8,
        .width = width,
        .height = height,
        .color_space = pl_color_space_bt2020_hlg,
        .icc_profile = load_icc_profile("invert-22.icc")
    };

    while(egl_app_frame())
    {
        // with colorspace
        color_engine_convert(&src, &cs);
        glViewport(0, 0, width, height);
        quad_draw(cstex);

        // without
        color_engine_convert(&src, &nocs);
        glViewport(0, height, width, height);
        quad_draw(nocstex);
    }

    return 0;
}
