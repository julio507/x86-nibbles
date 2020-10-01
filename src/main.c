#include <sys/io.h>
#include <multiboot.h>
#include <gfx/video.h>

#define PORT 0x3F8
#define STRING_END '\0'

#define BACKGROUND_COLOR 0x00FFFF
#define PLAYER_COLOR 0xFFFFFF
#define FRUIT_COLOR 0xFF0000
#define LEFT 0x4b
#define RIGHT 0x4d
#define UP 0x48
#define DOWN 0x50

int playerX = 10;
int playerY = 300;

int fruitX = 500;
int fruitY = 300;

int speed = 5;

char direction = 0;

char pk;

void usart_init(int base_addr)
{
    outb(base_addr + 1, 0x00);
    outb(base_addr + 3, 0x80);
    outb(base_addr + 0, 0x03);
    outb(base_addr + 1, 0x00);
    outb(base_addr + 3, 0x03);
    outb(base_addr + 2, 0xC7);
    outb(base_addr + 4, 0x0B);
}

void usart_write(int base_addr, unsigned char c)
{
    while( ( inb(base_addr + 5) & 0x20 ) == 0 );

    outb( base_addr, c );
}

void usart_puts(int base_addr, char *str)
{
    char c = *str;

    while( c != STRING_END )
    {
        usart_write( base_addr, c );

        str++;

        c = *str;
    }
}

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

void clear(int x, int y)
{
    draw_square(x, y, 10, 10, BACKGROUND_COLOR);
}

void isr0()
{
    clear(playerX, playerY);

    switch (direction)
    {
    case UP:
        playerY -= speed;
        break;

    case DOWN:
        playerY += speed;
        break;

    case LEFT:
        playerX -= speed;
        break;

    case RIGHT:
        playerX += speed;
        break;
    }

    draw_player(playerX, playerY);

    draw_fruit(fruitX, fruitY);
}

void isr1()
{
    char k = inb(0x60);

    if (k != pk)
    {
        pk = k;

        if (k == LEFT || k == RIGHT || k == UP || k == DOWN)
        {
            direction = k;
        }

        if (playerX == fruitX && playerY == fruitY)
        {
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

    usart_init( PORT );

    usart_write( PORT, 's' );
    usart_write( PORT, 'a' );
    usart_write( PORT, 'm' );
    usart_write( PORT, 'u' );
    usart_write( PORT, 'e' );
    usart_write( PORT, 'l' );


    usart_puts( PORT, " preguiçoso" + STRING_END );

    while (1)
    {
    }

    return 0;
}
