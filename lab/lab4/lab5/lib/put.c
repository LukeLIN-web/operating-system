#include"put.h"
int puts(const char *s)
{
    while (*s != '\0')
    {
        *UART16550A_DR = (unsigned char)(*s);
        s++;
    }
    return 0;
}
static char itoch(unsigned long x)
{
    if (x >= 0 && x <= 9)
    {
        return (char)(x + 48);
    }
    return 0;
}
void puti(unsigned long x)
{
    unsigned long digit = 1;
    unsigned long tmp = x;
    while (tmp >= 10)
    {
        digit *= 10;
        tmp /= 10;
    }
    while (digit >= 1)
    {
        *UART16550A_DR = (unsigned char)itoch(x/digit);
        x %= digit;
        digit /= 10;
    }
    return;
}
