
#pragma once

#include <wayland-client.h>
#include <wayland-egl.h>
#include <assert.h>
#include <stdio.h>

#include "shell.h"
#include "../gl/common.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

struct egl_app {
    struct wl_display *wl_display;
    struct wl_surface *wl_surface;
    struct wl_egl_window *egl_window;
    
    EGLDisplay egl_display;
    EGLConfig  egl_config;
    EGLSurface egl_surface;
    EGLContext egl_context;
} app;

static void egl_init(struct wl_display *display)
{
    EGLint major, minor, count, n;
    EGLConfig *configs;
    EGLBoolean ret;

    EGLint config_attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_SAMPLES, 16,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
	// EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    static const EGLint context_attribs[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE};

    app.egl_display = eglGetDisplay((EGLNativeDisplayType)display);
    //printf("app.egl_display: %p\n", app.egl_display);
    
    // set the current rendering API for this thread
    // ret = eglBindAPI(EGL_OPENGL_API);
    ret = eglBindAPI(EGL_OPENGL_ES_API);
    assert(ret == EGL_TRUE);

    ret = eglInitialize(app.egl_display, &major, &minor);
    assert(ret == EGL_TRUE);
    //printf("EGL major: %d, minor %d\n", major, minor);

    eglGetConfigs(app.egl_display, NULL, 0, &count);
    //printf("EGL has %d configs\n", count);

    configs = (void **)calloc(count, sizeof *configs);

    ret = eglChooseConfig(app.egl_display, config_attribs,
                    configs, count, &n);
    assert(ret && n >= 1);

    // just choose the first one
    app.egl_config = configs[0];

    app.egl_context = eglCreateContext(app.egl_display, app.egl_config,
        EGL_NO_CONTEXT, context_attribs);
    assert(app.egl_context);
}

static void egl_create_window(struct wl_surface *surface)
{
    app.egl_window = wl_egl_window_create(surface, 1000, 500);
    printf("app.egl_window: %p\n", app.egl_window);

    app.egl_surface = eglCreateWindowSurface(app.egl_display,
                               app.egl_config,
                               (EGLNativeWindowType)app.egl_window, NULL);
    assert(app.egl_surface != NULL);

    EGLBoolean ret = eglMakeCurrent(app.egl_display, app.egl_surface,
                       app.egl_surface, app.egl_context);
    assert(ret);
 
    // If we set the interval to 0, don't limit framerate to refresh rate.
    // This will still happen if we rely on wl_callback_frame()
    eglSwapInterval(app.egl_display, 0);
}

void egl_app_begin()
{
    app.wl_display = wl_display_connect(NULL);
    shell_init(app.wl_display);
    app.wl_surface = shell_create_surface();

    egl_init(shell.display);
    egl_create_window(app.wl_surface);
}

// return true when we are ready to draw again
// return false on error
bool egl_app_frame()
{
    eglSwapBuffers(app.egl_display, app.egl_surface);
    if(wl_display_dispatch(app.wl_display) == -1)
        return false;

    wl_egl_window_resize(app.egl_window, shell.width, shell.height, 0, 0);
    return true;
}
