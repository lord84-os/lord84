#include <stdint.h>

#pragma once

typedef enum proc_state {
    RUNNING,
    READY,
    SLEEPING,
    UNUSED = 0
}proc_state;

typedef struct context {
    uint64_t rbx, rsp, rbp, r12, r13, r14, r15;
    uint64_t rip, rflags;
} __attribute((packed))context;

typedef struct proc {
    uint64_t *mem;
    uint64_t *kstack;
    proc_state state;
    uint16_t pid;
    context context;
}proc;

void scheduler_init();

#define PROC_MAX    512 // Max number of processes

#define INITIAL_STACK_SIZE  0x10000

