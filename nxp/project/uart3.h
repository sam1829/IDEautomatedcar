#ifndef  UART3_H
#define  UART3_H
#include  <stdint.h>
void uart3_put(char *ptr_str);
void uart3_init(void);
uint8_t uart3_getchar(void);
void uart3_putchar(char ch);
#endif  /*  ifndef  UART3_H  */
