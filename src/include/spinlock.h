#include <stdatomic.h>

#ifndef SPINLOCK_H
#define SPINLOCK_H

atomic_flag example_lock_variable = ATOMIC_FLAG_INIT;

void acquire_lock(atomic_flag *lock){
    while(atomic_flag_test_and_set_explicit(lock, memory_order_acquire)){
        __builtin_ia32_pause();
    }
}

void release_lock(atomic_flag *lock){
    atomic_flag_clear_explicit(lock, memory_order_release);
}

#endif