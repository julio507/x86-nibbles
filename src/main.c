#include <sys/io.h>
#include <multiboot.h>
#include <gfx/video.h>

#define PORT1 0x3F8
#define PORT2 0x2F8

#define STRING_END '\0'

#define BACKGROUND_COLOR 0x00FFFF
#define PLAYER_COLOR1 0xFFFFFF
#define PLAYER_COLOR2 0x008000
#define FRUIT_COLOR 0xFF0000
#define SCORE_COLOR1 0xFFFF00
#define SCORE_COLOR2 0xFFA500
#define LEFT 0x4b
#define RIGHT 0x4d
#define UP 0x48
#define DOWN 0x50

#define NONE 0
#define SERVER 1
#define CLIENT 2

#define MESSAGE_START 0xFF

int mode = NONE;

int score1 = 0;
int score2 = 0;

int playerX1 = 10;
int playerY1 = 300;

int playerX2 = 100;
int playerY2 = 300;

int fruitX = 500;
int fruitY = 300;

int speed = 5;

char direction1 = 0;
char direction2 = 0;

char pk;

int positionsX[] = {500, 250, 100, 600};
int positionsy[] = {300, 400, 200, 260};

int vectorX1[100];
int vectorY1[100];
int directions1[100];

int vectorX2[100];
int vectorY2[100];
int directions2[100];

unsigned int p = 0;

void usart_init(int base_addr)
{
    outb(base_addr + 1, 0x00);
    outb(base_addr + 3, 0x80);
    outb(base_addr + 0, 0x01);
    outb(base_addr + 1, 0x00);
    outb(base_addr + 3, 0x03);
    outb(base_addr + 2, 0xC7);
    outb(base_addr + 4, 0x0B);
}

void usart_write(int base_addr, unsigned char c)
{
    while ((inb(base_addr + 5) & 0x20) == 0)
    {
    };

    outb(base_addr, c);
}

void usart_puts(int base_addr, char *str)
{
    char c = *str;

    while (c != STRING_END)
    {
        usart_write(base_addr, c);

        str++;

        c = *str;
    }
}

void draw_board(void)
{
    draw_square(0, 0, 800, 600, BACKGROUND_COLOR);
}

void draw_player(int x, int y, int color)
{
    draw_square(x, y, 10, 10, color);
}

void draw_fruit(int x, int y)
{
    draw_square(x, y, 10, 10, FRUIT_COLOR);
}

void drawn_score( int s, int pad, int color )
{
    for (int i = 0; i < s; i++)
    {
        draw_square(10 * i, 10 + pad, 5, 5, color);
    }
}

void clear(int x, int y)
{
    draw_square(x, y, 10, 10, BACKGROUND_COLOR);
}

void player(int color, int pX, int pY, int vX[], int vY[], int vD[], char d, int s)
{
    for (int c = 0; c <= s; c++)
    {
        switch (vD[c])
        {
        case UP:
            clear(vX[c], vY[c] + speed);
            break;

        case DOWN:
            clear(vX[c], vY[c] - speed);
            break;

        case LEFT:
            clear(vX[c] + speed, vY[c]);
            break;

        case RIGHT:
            clear(vX[c] - speed, vY[c]);
            break;
        }
    }

    switch (d)
    {
    case UP:
        pY -= speed;
        break;

    case DOWN:
        pY += speed;
        break;

    case LEFT:
        pX -= speed;
        break;

    case RIGHT:
        pX += speed;
        break;
    }

    vD[s] = d;

    vX[s] = pX;
    vY[s] = pY;

    for (int c = 0; c <= s; c++)
    {
        draw_player(vX[c], vY[c], color);

        if (c < s)
        {
            vD[c] = vD[c + 1];

            vX[c] = vX[c + 1];
            vY[c] = vY[c + 1];
        }
    }
}

void check_score(int pX, int pY, int s)
{
    if (pX + 10 >= fruitX && pX <= fruitX + 10 && pY + 10 >= fruitY && pY <= fruitY + 10)
    {
        clear(fruitX, fruitY);

        s++;

        p++;

        if (p > (sizeof(positionsX) / sizeof(int)))
        {
            p = 0;
        }

        fruitX = positionsX[p];
        fruitY = positionsy[p];
    }
}

void isr0()
{
    player(PLAYER_COLOR1, playerX1, playerY1, vectorX1, vectorY1, directions1, direction1, score1);

    player(PLAYER_COLOR2, playerX2, playerY2, vectorX2, vectorY2, directions2, direction2, score2);

    draw_fruit(fruitX, fruitY);

    drawn_score( score1, 0, SCORE_COLOR1 );

    drawn_score( score2, 5, SCORE_COLOR2 );

    if( mode == CLIENT )
    {
        usart_write( PORT2, MESSAGE_START );
        usart_write( PORT2, direction2 );
    }

    else if( mode == SERVER )
    {
        check_score(playerX1, playerY1, score1);

        check_score(playerX2, playerY2, score2);

        usart_write( PORT2, MESSAGE_START );
        usart_write( PORT2, direction1 );
        usart_write( PORT2, score1 );
        usart_write( PORT2, score2 );
        usart_write( PORT2, fruitX );
        usart_write( PORT2, fruitY );
    }
    
}

void isr1()
{
    if( mode == NONE )
    {
        mode = SERVER;        
    }

    char k = inb(0x60);

    if (k != pk)
    {
        pk = k;

        if (k == LEFT || k == RIGHT || k == UP || k == DOWN)
        {
            if( mode == SERVER )
            {
                direction1 = k;
            }

            else if( mode == CLIENT ) 
            {
                direction2 = k;
            }
        }
    }
}

void isr3()
{
    if (mode == NONE)
    {
        mode = CLIENT;
    }

    if (mode == CLIENT)
    {
        if( inb( PORT2 ) == MESSAGE_START )
        {
            direction1 = inb( PORT2 );
            score1 = inb( PORT2 );
            score2 = inb( PORT2 );
            fruitX = inb( PORT2 );
            fruitY = inb( PORT2 );
        }
    }

    else if( mode == SERVER )
    {
        direction2 = inb( PORT2 );
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

    usart_init(PORT1);
    usart_init(PORT2);

    usart_write(PORT1, 's');
    usart_write(PORT1, 'a');
    usart_write(PORT1, 'm');
    usart_write(PORT1, 'u');
    usart_write(PORT1, 'e');
    usart_write(PORT1, 'l');

    usart_puts(PORT1, " preguiçoso" + STRING_END);

    while (1)
    {
    }

    return 0;
}
