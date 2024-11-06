#include <stdint.h>
void ioapic_init(void);
void write_redir_entry(uint8_t reg, uint64_t data);

#define IOREGSEL    0x0
#define IOWIN       0x10

#define IOAPICID    0x0
#define IOAPICVER   0x1
#define IOAPICARB   0x2
#define IOREDTBL(x) (0x10 + (x * 2)) // 0-23 registers