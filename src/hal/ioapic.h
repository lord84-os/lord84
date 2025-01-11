#include "error.h"
#include <stdint.h>
void ioapic_init(void);
void write_redir_entry(uint8_t reg, uint64_t data);
kstatus set_redir_entry(uint8_t pin, uint8_t vector, uint8_t delivery, uint8_t trigger, uint8_t destination_field, uint8_t destination_mode);

#define IOREGSEL    0x0
#define IOWIN       0x10

#define IOAPICID    0x0
#define IOAPICVER   0x1
#define IOAPICARB   0x2
#define IOREDTBL(x) (0x10 + (x * 2)) // 0-23 registers