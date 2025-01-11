#include <stdint.h>

enum USABLE_TIMERS {
    HPET = 0,
    PMT,
    PIT,
};

void timer_init(void);
void apic_timer_handler(void);
void sleep(int ms);