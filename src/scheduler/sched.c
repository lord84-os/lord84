#include <stdio.h>
#include <string.h>
#include <lord84.h>
#include "../hal/smp.h"
#include <error.h>
#include "../mm/kmalloc.h"
#include "sched.h"

/*
    Per CPU round robin scheduler.
*/

extern context *save_context();

extern void switch_context(context *old, context *new);

#define QUANTUM_US 10000

void idle_task(){
    kprintf("hi\n");
    for(;;);
}

/* Initializes scheduler for calling CPU */
void sched_init(){
/*     proc *list = get_cpu_struct()->process_list;
    list = kmalloc(sizeof(proc));
    get_cpu_struct()->process_list = list;

    if(!list){
        klog(LOG_ERROR, __func__, "Failed to allocate head process");
        kkill();
    }

    memset(list, 0, sizeof(proc));

    list[0].context.rip = (uint64_t)&idle_task;
    list[0].state = READY;
    get_cpu_struct()->current_process = &list[0]; */
}

/* Entry back into kernel code */
void sched_entry(){
/*     cpu_state *state = get_cpu_struct();
    for(;;){
        asm("sti");

        proc *next_process = state->current_process->next;

        if(!next_process){
            next_process = &state->process_list[0];
            break;
        }

        state->current_process->state = READY;
        
        int i = 0;
        while(i < PROC_MAX){

            if(next_process->state == READY){
                break;
            }

            next_process = next_process->next;

            if(!next_process){
                next_process = &state->process_list[0];
                break;
            }

            i++;
        }

        next_process->state = RUNNING;
        
        state->current_process = next_process;

        switch_context(&state->scheduler_context, &next_process->context);

        state->current_process->state = READY;
    } */
}

