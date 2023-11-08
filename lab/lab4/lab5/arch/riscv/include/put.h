#ifndef PUT_H
#define PUT_H
#define UART16550A_DR (volatile unsigned char *)0xffffffdf90000000
void puti(unsigned long num);
int puts(const char *s);
#endif
