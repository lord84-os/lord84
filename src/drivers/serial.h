#include <stdint.h>

void serial_write(uint8_t data);
uint8_t serial_read();

void serial_print(char *str);

void serial_init();