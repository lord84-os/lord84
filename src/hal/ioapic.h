#include <stdint.h>
void ioapic_init(void);
void write_redir_entry(uint8_t reg, uint64_t data);