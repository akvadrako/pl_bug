// easy wayland wrapper
// xdg-shell usage

#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <wayland-client.h>

#include "xdg-shell.h"

bool configured = false;
bool allow_scale = false;

typedef struct {
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct wl_surface *surface;
    struct wl_output *output;
    struct xdg_wm_base *base;
    int scale;
    int width;
    int height;
} shell_t;

shell_t shell;

struct xdg_toplevel *toplevel;
struct xdg_surface *xdg_surface;

static void info(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}

static void
shell_handle_ping(void *data, struct xdg_wm_base *base, uint32_t serial)
{
    xdg_wm_base_pong(base, serial);
    info("ping pong\n");
}

static void handle_output_scale(void *data, struct wl_output *output, int32_t scale) {
    printf("scale: %p %d honor=%i\n", output, scale, allow_scale);
    if(allow_scale) {
        if(scale > shell.scale) {
            shell.scale = scale;
        }
    }
}


static void handle_output_geometry(void *data, struct wl_output *output,
                                   int32_t x, int32_t y, int32_t physical_width,
                                   int32_t physical_height, int32_t subpixel,
                                   const char *make, const char *model,
                                   int32_t output_transform) {
  //printf("output: %p geo\n", output);
}

static void handle_output_mode(void *data, struct wl_output *output,
                               uint32_t flags, int32_t width, int32_t height,
                               int32_t refresh) {
  //printf("output: %p mode\n", output);
}

static void handle_output_done(void *data, struct wl_output *output) {
  //printf("output: %p done\n", output);
}

static void
shell_registry_handler(void *data, struct wl_registry *registry, uint32_t id,
                       const char *interface, uint32_t version)
{
    if (strcmp(interface, "wl_compositor") == 0)
    {
        shell.compositor = (struct wl_compositor *)wl_registry_bind(registry, id, &wl_compositor_interface, version);
    }
    else if (strcmp(interface, "wl_output") == 0)
    {
        struct wl_output *output = (struct wl_output *)wl_registry_bind(registry, id, &wl_output_interface, version);
        printf("output: %p new\n", output);
        static struct wl_output_listener output_listener = {
                handle_output_geometry,
                handle_output_mode,
                handle_output_done,
                handle_output_scale,
        };
        wl_output_add_listener(output, &output_listener, NULL);
    }
    else if (strcmp(interface, "xdg_wm_base") == 0)
    {
        shell.base = (struct xdg_wm_base *)wl_registry_bind(registry, id, &xdg_wm_base_interface, version);
        static struct xdg_wm_base_listener listener = {shell_handle_ping};
        xdg_wm_base_add_listener(shell.base, &listener, NULL);
    }
}

static void
shell_registry_remover(void *data, struct wl_registry *registry, uint32_t id)
{
    info("Got a registry losing event for %d\n", id);
}

static void shell_init(struct wl_display *display)
{
    shell.display = display;
    shell.scale = 1;

    static struct wl_registry_listener listener = {
        shell_registry_handler,
        shell_registry_remover,
    };
    wl_registry_add_listener(wl_display_get_registry(shell.display), &listener, NULL);

    wl_display_dispatch(shell.display);
    wl_display_roundtrip(shell.display);

    info("wl_compositor: %p\n", shell.compositor);
    info("xdg_wm_base: %p\n", shell.base);
}

void handle_surface_enter(void *data, struct wl_surface *surface, struct wl_output *output)
{
    info("handle_surface_enter: %p %p\n", surface, output);
    shell.output = output;
}

void handle_surface_leave(void *data, struct wl_surface *surface, struct wl_output *output)
{
    info("handle_surface_leave: %p %p\n", surface, output);
}

static void
handle_xdg_surface_configure(void *data, struct xdg_surface *surface,
                             uint32_t serial)
{
    xdg_surface_ack_configure(surface, serial);
    configured = true;
}

static void
handle_xdg_toplevel_configure(void *data, struct xdg_toplevel *xdg_toplevel,
                              int32_t width, int32_t height,
                              struct wl_array *state)
{
    info("handle_xdg_toplevel_configure: %ix%i\n", width, height);
    shell.width = shell.scale * width;
    shell.height = shell.scale * height;
}

static void
handle_xdg_toplevel_close(void *data, struct xdg_toplevel *xdg_toplevel)
{
    info("handle_xdg_toplevel_close\n");
}

static struct wl_surface *shell_create_surface()
{
    struct wl_surface *surface = wl_compositor_create_surface(shell.compositor);
    info("wl_surface: %p\n", surface);
  
    wl_surface_set_buffer_scale(surface, shell.scale);

    static struct wl_surface_listener surface_listener = {handle_surface_enter, handle_surface_leave};
    wl_surface_add_listener(surface, &surface_listener, NULL);

    xdg_surface = xdg_wm_base_get_xdg_surface(shell.base, surface);
    info("xdg_surface: %p\n", xdg_surface);

    static struct xdg_surface_listener config_listener = {handle_xdg_surface_configure};
    xdg_surface_add_listener(xdg_surface, &config_listener, NULL);

    toplevel = xdg_surface_get_toplevel(xdg_surface);

    static struct xdg_toplevel_listener toplevel_listener = {
        handle_xdg_toplevel_configure,
        handle_xdg_toplevel_close,
    };
    xdg_toplevel_add_listener(toplevel, &toplevel_listener, NULL);

    wl_surface_commit(surface);

    info("wait for surface to be configured\n");
    while (!configured && wl_display_dispatch(shell.display) != -1)
    {
        info("wait\n");
    }

    return surface;
}

