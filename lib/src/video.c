#include <gfx/video.h>

static unsigned int *vidptr;
static unsigned int vidwidth;
static unsigned int vidheight;

void screen_init(unsigned long addr, unsigned int width, unsigned int height)
{
    vidptr = (unsigned int *)addr;
    vidwidth = width;
    vidheight = height;
}

void draw_square(int x, int y, int w, int h, int c)
{
    int i, j;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            vidptr[y * vidwidth + x + i * vidwidth + j] = c;
        }
    }
}

