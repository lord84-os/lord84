#include <stdbool.h>
#include <stdint.h>
#include "../scheduler/sched.h"

#pragma once

#define GSBASE 0xC0000101
#define KERNELGSBASE 0xC0000102

typedef struct cpu_state {
    uint32_t lapic_id;
    uint64_t lapic_timer_ticks;
    proc process_list[PROC_MAX];
    proc *current_process;
    uint16_t process_count;
    context scheduler_context;
}__attribute((packed))cpu_state;

void smp_init();
cpu_state *get_cpu_struct();
uint64_t get_cpu_count();
bool get_cpu_struct_initialized();

