#include <stdatomic.h>

#ifndef SPINLOCK_H
#define SPINLOCK_H

void acquire_lock(atomic_flag *lock);
void free_lock(atomic_flag *lock);

#endif