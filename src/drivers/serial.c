#include "../sys/acpi.h"
#include "../hal/ioapic.h"
#include <io.h>
#include <stdio.h>

#define COM1            0x3F8

#define LINE_CTRL_REG   0x3
#define LINE_STAT_REG   0x5
#define INT_ENABLE_REG  0x1
#define MDM_CTRL_REG    0x4

#define DIVISOR         0x12     // We want a baud rate of 9600, so the divisor here is 12

bool serial_enabled = false;

void serial_init(){

    /* Disable interrupts */
    outb(COM1 + INT_ENABLE_REG, 0);

    /* Set the DLAB bit */
    outb(COM1 + LINE_CTRL_REG, (1 << 7));

    /* Send least significant byte of divisor */
    outb(COM1 + 1, 0);

    /* Send most significant byte of divisor */
    outb(COM1, 12);

    /* Clear the DLAB bit */
    outb(COM1 + LINE_CTRL_REG, (0 << 7));

    /* Set the character length to 8 bits, parity to none and stop bit to 1 */
    outb(COM1 + LINE_CTRL_REG, 0b00000011);

    /* Set DTR, RTS and enables IRQ */
    outb(COM1 + MDM_CTRL_REG, 0b00001101);

    /* Set loopback testing mode to see if UART werks */
    outb(COM1 + MDM_CTRL_REG, 0b00011110);

    outb(COM1, 0xAE);

    if(inb(COM1) != 0xAE){
        klog(LOG_WARN, __func__, "Serial controller failed test, serial output will not work");
        return;
    }

    /* Disable loopback and set DTR bit */
    outb(COM1 + MDM_CTRL_REG, 0b00001111);

    serial_enabled = true;

}

uint8_t serial_read(){
    while((inb(COM1 + LINE_STAT_REG) & 0x1) == 0){ asm("nop"); }

    return inb(COM1);
}

void serial_write(uint8_t data){
    while((inb(COM1 + LINE_STAT_REG) & (1 << 5)) == 0){ asm("nop"); }
    
    outb(COM1, data);
}

void serial_print(char *str){
    uint64_t i = 0;
    while (str[i] != '\0') {
        serial_write(str[i]);
        i++;
    }
}

