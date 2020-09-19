#include <sys/io.h>
#include <multiboot.h>
#include <gfx/video.h>

#define BACKGROUND_COLOR 0x00FFFF
#define PLAYER_COLOR 0x000000
#define FRUIT_COLOR 0xFF0000

int playerX = 10;
int playerY = 300;

int fruitX = 500;
int fruitY = 300;

char pk;

void draw_board(void)
{
    draw_square(0, 0, 800, 600, BACKGROUND_COLOR);
}

void draw_player(int x, int y)
{
    draw_square(x, y, 10, 10, PLAYER_COLOR);
}

void draw_fruit(int x, int y)
{
    draw_square(x, y, 10, 10, FRUIT_COLOR);
}

void isr1()
{
    char k = inb(0x60);

    if (k != pk)
    {
        pk = k;

        switch (k)
        {
        case 0x48:
            draw_square(playerX, playerY - 10, 10, 10, BACKGROUND_COLOR);
            draw_player(playerX, playerY);
            break;

        case 0x50:
            draw_square(playerX, playerY++, 10, 10, BACKGROUND_COLOR);
            draw_player(playerX, playerY);
            break;

        case 0x4b:
            draw_square(playerX--, playerY, 10, 10, BACKGROUND_COLOR);
            draw_player(playerX, playerY);
            break;

        case 0x4d:
            draw_square(playerX++, playerY, 10, 10, BACKGROUND_COLOR);
            draw_player(playerX, playerY);
            break;
        }
    }
}

int main(unsigned long addr)
{
    multiboot_info_t *mbi;

    mbi = (multiboot_info_t *)addr;

    screen_init(mbi->framebuffer_addr,
                mbi->framebuffer_width,
                mbi->framebuffer_height);

    draw_board();

    draw_player(playerX, playerY);

    draw_fruit(fruitX, fruitY);

    while (1)
    {
    }

    return 0;
}
