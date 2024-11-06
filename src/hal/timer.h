#include <stdint.h>
void timer_init(void);
int hpet_init(void);
void hpet_handler(void);
void hpet_sleep(uint64_t ms);