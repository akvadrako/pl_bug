// OpenGL Color conversion

#pragma once

#include <stdio.h>
#include <assert.h>

#include <libplacebo/opengl.h>
#include <libplacebo/renderer.h>
#include <libplacebo/gpu.h>

#include "common.h"
#include "util/file.h"

struct color_texture {
    int width, height;
    int sample_depth; // total bits per color
    int color_depth;  // used bits per color
    struct pl_color_space color_space;
    struct pl_icc_profile icc_profile;
    GLuint texture_id;
    GLuint fbo;
};

struct color_engine {
    // libplacebo state
    struct pl_context *ctx;
    struct pl_renderer *renderer;
    const struct pl_gpu *gpu;
    const struct pl_opengl *gl;
};

struct color_engine *engine = &(struct color_engine){0};

void color_engine_init()
{
    engine->ctx = pl_context_create(PL_API_VER, &(struct pl_context_params) {
            .log_cb    = pl_log_color,
            .log_level = PL_LOG_DEBUG,
            });

    assert(engine->ctx);

    engine->gl = pl_opengl_create(engine->ctx, &(struct pl_opengl_params){
            .debug = true,
        });
    assert(engine->gl);
    engine->gpu = engine->gl->gpu;

    engine->renderer = pl_renderer_create(engine->ctx, engine->gpu);
    assert(engine->renderer);
}

struct pl_icc_profile load_icc_profile(const char *path)
{
    struct file data;
    bool ok = open_file(path, &data);
    assert(ok);

    return (struct pl_icc_profile) {
        .data = data.data,
        .len = data.size,
        .signature = (uint64_t)data.data,
    };
}

const struct pl_tex *get_pl_texture(struct color_texture *tex)
{
    return pl_opengl_wrap(engine->gpu, &(struct pl_opengl_wrap_params){
        .width = tex->width, 
        .height = tex->height,
        .depth = 0,
        .target = GL_TEXTURE_2D,
        // works on GLES3: GL_RGBA8, GL_RGBA16F
        // works on GL4: GL_RGBA32F
        .iformat = GL_RGBA16F,
        .texture = tex->texture_id,
    });
}

struct pl_color_repr get_pl_repr(struct color_texture *tex)
{
    return (struct pl_color_repr) {
        .sys = PL_COLOR_SYSTEM_RGB,
        .levels = PL_COLOR_LEVELS_PC, // PL_COLOR_LEVELS_TV = 16-235
        .alpha = PL_ALPHA_UNKNOWN,
        .bits = (struct pl_bit_encoding) {
            .sample_depth = tex->sample_depth,
            .color_depth = tex->color_depth,
            .bit_shift = 0,
        },
    };
}

void color_engine_convert(struct color_texture *src, struct color_texture *dest)
{
    struct pl_image image = {
        .num_planes = 1,
        .planes     = {{
            .components = 4,
            .component_mapping = {0,1,2,3},
            .texture = get_pl_texture(src),
        }},
        .repr       = get_pl_repr(src),
        .color      = src->color_space,
        .profile    = src->icc_profile,
        .src_rect   = {0, 0, src->width, src->height},
    };
    
    struct pl_render_target target = {
        .fbo        = get_pl_texture(dest),
        .repr       = get_pl_repr(dest),
        .color      = dest->color_space,
        .profile    = dest->icc_profile,
        .dst_rect   = {0, 0, dest->width, dest->height},
    };

    bool ok = pl_render_image(engine->renderer, &image, &target, &(struct pl_render_params){0});
    assert(ok);
}


