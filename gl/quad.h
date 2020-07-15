// OpenGL - draw a texture on a rectangle

#pragma once

#include <stdio.h>
#include <assert.h>

#include "common.h"

GLuint quad_vao;
GLuint quad_program;
GLuint quad_sampler_loc;

GLuint create_quad_vao() {
    GLuint vao = 0, vbo = 0;
    float verts[] = {
        -1.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 1.0f,
        0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f };
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, 16 * sizeof( float ), verts, GL_STATIC_DRAW );

    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );
    glEnableVertexAttribArray( 0 );
    GLintptr stride = 4 * sizeof( float );
    glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, stride, NULL );
    glEnableVertexAttribArray( 1 );
    GLintptr offset = 2 * sizeof( float );
    glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid *)offset );
    
    glBindVertexArray(0);

    return vao;
}

GLuint create_quad_program() {
    GLuint vert_shader = gl_create_shader(R"""(
        #version 320 es
        layout (location = 0) in vec2 vp;
        layout (location = 1) in vec2 vt;
        out vec2 st;

        void main () {
            st = vt;
            gl_Position = vec4 (vp, 0.0, 1.0);
        }
    )""", GL_VERTEX_SHADER);

    GLuint frag_shader = gl_create_shader(R"""(
        #version 320 es
        precision mediump float;
        in vec2 st;
        uniform sampler2D img;
        out vec4 fc;

        void main () {
            // ignore alpha
            fc = vec4(texture2D(img, st).rgb, 1.0);
            // respect alpha
            // fc = texture (img, st);
        }
    )""", GL_FRAGMENT_SHADER);

    GLuint program = glCreateProgram();
    glAttachShader( program, vert_shader );
    glAttachShader( program, frag_shader );
    glLinkProgram( program );
    quad_sampler_loc = glGetUniformLocation(program, "img");
    return program;
}

void quad_init()
{
    quad_vao = create_quad_vao();
    quad_program = create_quad_program();
}

void quad_draw(GLuint tex_output)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glUseProgram( quad_program );
    glBindVertexArray( quad_vao );
    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, tex_output );
    
    glUniform1i(quad_sampler_loc, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    
    // cleanup
    glUseProgram(0);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
