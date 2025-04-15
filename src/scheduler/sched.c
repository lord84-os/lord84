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
    for(;;);
}

/* Initializes scheduler for calling CPU */
void sched_init(){
    proc *list = get_cpu_struct()->process_list;
    list = kmalloc(sizeof(proc));

    if(!list){
        klog(LOG_ERROR, __func__, "Failed to allocate head process");
        
    }
}

void sched_entry(){
    for(;;){

    }
}

