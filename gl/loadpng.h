
#pragma once

#include "../util/lodepng.h"
#include "../gl/common.h"

static GLuint loadpng_texture(char *path)
{
    uint32_t img_w;
    uint32_t img_h;

    uint8_t *img;
    if(!lodepng_decode32_file(&img, &img_w, &img_h, path))
        fprintf(stderr, "read png error\n");

    GLuint tex;
    glGenTextures(1, &tex);
    
    glBindTexture(GL_TEXTURE_2D, tex);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img_w, img_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

    return tex;
}

