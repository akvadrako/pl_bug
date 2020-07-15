
#pragma once

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct file
{
    void *data;
    size_t size;
};

static bool open_file(const char *path, struct file *out)
{
    FILE *fp = NULL;
    bool success = false;

    fp = fopen(path, "rb");
    if (!fp)
        goto done;

    if (fseeko(fp, 0, SEEK_END))
        goto done;
    off_t size = ftello(fp);
    if (size < 0)
        goto done;
    if (fseeko(fp, 0, SEEK_SET))
        goto done;

    void *data = malloc(size);
    if (!fread(data, size, 1, fp))
        goto done;

    *out = (struct file) {
        .data = data,
        .size = size,
    };

    success = true;
done:
    if (fp)
        fclose(fp);
    return success;
}

