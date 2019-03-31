#ifndef  UART_H
#define  UART_H
#include  <stdint.h>
void uart0_put(char *ptr_str);
void uart0_init(void);
uint8_t uart0_getchar(void);
void uart0_putchar(char ch);
#endif  /*  ifndef  UART_H  */
