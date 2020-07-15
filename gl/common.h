
#pragma once

#include <GLES3/gl32.h>
#include <GLES3/gl3ext.h>

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

static GLuint gl_create_shader(const char *source, GLenum shader_type)
{
    GLuint shader;
    GLint status;

    shader = glCreateShader(shader_type);
    assert(shader != 0);

    glShaderSource(shader, 1, (const char **) &source, NULL);
    glCompileShader(shader);

    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[1000];
        GLsizei len;
        glGetShaderInfoLog(shader, 1000, &len, log);
        fprintf(stderr, "Error: compiling %s: %.*s\n",
                shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
                len, log);
    }

    return shader;
}

GLuint gl_create_2d_texture(GLuint format, int width, int height)
{
    GLuint tex;
    glGenTextures(1, &tex);
    
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, tex);
    
    // Give an empty image to OpenGL
    switch(format) {
        case GL_RGBA8:
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
            break;
        case GL_RGBA16F:
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
            break;
        case GL_UNSIGNED_SHORT_5_6_5:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
            break;
        default:
            assert("unknown format" == 0);
    }
    
    // Needed. GL_LINEAR or GL_NEAREST
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return tex;
}

