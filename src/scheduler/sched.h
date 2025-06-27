#include <stdint.h>

#pragma once

typedef enum proc_state {
    RUNNING,
    READY,
    SLEEPING,
    UNUSED = 0
}proc_state;

typedef struct context {
    uint64_t r15, r14, r13, r12, rbx, rbp, rsp;
    uint64_t rip, rflags;
} __attribute((packed))context;

typedef struct thread {
    uint64_t *mem;
    uint64_t *kstack;
    proc_state state;
    uint16_t pid;
    context context;
}thread;

typedef struct proc {
    uint64_t *mem;
    uint64_t *kstack;
    proc_state state;
    thread *threads;
    uint16_t pid;
    context context;
    struct proc *next;
}proc;


void sched_init();
void sched_entry();

#define PROC_MAX    512 // Max number of processes

#define INITIAL_STACK_SIZE  0x10000

