
#include <stdint.h>

struct ipi {
    uint8_t vector;
    uint32_t delivery_mode : 3;
    uint32_t destination_mode : 1;
    uint32_t level : 1;
    uint32_t destination_sh : 2;
};

void apic_init(void);
void ap_apic_init();
void apic_sleep(uint64_t ms);
void apic_send_ipi(struct ipi ipi);

