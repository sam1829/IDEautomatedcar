#ifndef  UART3_H
#define  UART3_H
#include  <stdint.h>
void uart_put3(char *ptr_str);
void uart_init3(void);
uint8_t uart_getchar3(void);
void uart_putchar3(char ch);
#endif  /*  ifndef  UART3_H  */
