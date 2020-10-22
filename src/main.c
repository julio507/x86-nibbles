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
    outb(base_addr + 1, 0x00); // Disable all interrupts
    outb(base_addr + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(base_addr + 0, 0x01); // Set divisor to 1 (lo byte) 115200 baud
    outb(base_addr + 1, 0x00); //                  (hi byte)
    outb(base_addr + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(base_addr + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
    outb(base_addr + 4, 0x0B); // IRQs enabled, RTS/DSR set
    outb(base_addr + 1, 0x01); // Enable all interrupts
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

void drawn_score(int s, int pad, int color)
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

void player1()
{
    for (int c = 0; c <= score1; c++)
    {
        switch (directions1[c])
        {
        case UP:
            clear(vectorX1[c], vectorY1[c] + speed);
            break;

        case DOWN:
            clear(vectorX1[c], vectorY1[c] - speed);
            break;

        case LEFT:
            clear(vectorX1[c] + speed, vectorY1[c]);
            break;

        case RIGHT:
            clear(vectorX1[c] - speed, vectorY1[c]);
            break;
        }
    }

    switch (direction1)
    {
    case UP:
        playerY1 -= speed;
        break;

    case DOWN:
        playerY1 += speed;
        break;

    case LEFT:
        playerX1 -= speed;
        break;

    case RIGHT:
        playerX1 += speed;
        break;
    }

    directions1[score1] = direction1;

    vectorX1[score1] = playerX1;
    vectorY1[score1] = playerY1;

    for (int c = 0; c <= score1; c++)
    {
        draw_player(vectorX1[c], vectorY1[c], PLAYER_COLOR1);

        if (c < score1)
        {
            directions1[c] = directions1[c + 1];

            vectorX1[c] = vectorX1[c + 1];
            vectorY1[c] = vectorY1[c + 1];
        }
    }
}

void player2()
{
    for (int c = 0; c <= score2; c++)
    {
        switch (directions2[c])
        {
        case UP:
            clear(vectorX2[c], vectorY2[c] + speed);
            break;

        case DOWN:
            clear(vectorX2[c], vectorY2[c] - speed);
            break;

        case LEFT:
            clear(vectorX2[c] + speed, vectorY2[c]);
            break;

        case RIGHT:
            clear(vectorX2[c] - speed, vectorY2[c]);
            break;
        }
    }

    switch (direction2)
    {
    case UP:
        playerY2 -= speed;
        break;

    case DOWN:
        playerY2 += speed;
        break;

    case LEFT:
        playerX2 -= speed;
        break;

    case RIGHT:
        playerX2 += speed;
        break;
    }

    directions2[score2] = direction2;

    vectorX2[score2] = playerX2;
    vectorY2[score2] = playerY2;

    for (int c = 0; c <= score2; c++)
    {
        draw_player(vectorX2[c], vectorY2[c], PLAYER_COLOR2);

        if (c < score2)
        {
            directions2[c] = directions2[c + 1];

            vectorX2[c] = vectorX2[c + 1];
            vectorY2[c] = vectorY2[c + 1];
        }
    }
}

void check_score1()
{
    if (playerX1 + 10 >= fruitX && playerX1 <= fruitX + 10 && playerY1 + 10 >= fruitY && playerY1 <= fruitY + 10)
    {
        clear(fruitX, fruitY);

        score1++;

        p++;

        if (p > (sizeof(positionsX) / sizeof(int)))
        {
            p = 0;
        }

        fruitX = positionsX[p];
        fruitY = positionsy[p];
    }
}

void check_score2()
{
    if (playerX2 + 10 >= fruitX && playerX2 <= fruitX + 10 && playerY2 + 10 >= fruitY && playerY2 <= fruitY + 10)
    {
        clear(fruitX, fruitY);

        score2++;

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
    player1();
    player2();

    draw_fruit(fruitX, fruitY);

    drawn_score(score1, 0, SCORE_COLOR1);

    drawn_score(score2, 5, SCORE_COLOR2);

    check_score1();

    check_score2();
}

void isr1()
{
    if (mode == NONE)
    {
        mode = SERVER;
        usart_puts(PORT1, "\nserver mode" + STRING_END);
    }

    char k = inb(0x60);

    if (k != pk)
    {
        pk = k;

        if (k == LEFT || k == RIGHT || k == UP || k == DOWN)
        {
            if (mode == SERVER)
            {
                direction1 = k;
            }

            else if (mode == CLIENT)
            {
                direction2 = k;
            }
        }

        if (mode == CLIENT)
        {
            usart_write(PORT2, MESSAGE_START);
            usart_write(PORT2, direction2);
        }

        else if (mode == SERVER)
        {
            usart_write(PORT2, MESSAGE_START);
            usart_write(PORT2, direction1);
        }
    }
}

void isr3()
{
    if (mode == NONE)
    {
        mode = CLIENT;
        usart_puts(PORT1, "\nclient mode" + STRING_END);
    }

    if (mode == CLIENT)
    {
        if (inb(PORT2) == MESSAGE_START)
        {
            direction1 = inb(PORT2);
        }
    }

    else if (mode == SERVER)
    {
        if (inb(PORT2) == MESSAGE_START)
        {
            direction2 = inb(PORT2);
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
