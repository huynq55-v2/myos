#ifndef SPINLOCK_H
#define SPINLOCK_H

typedef struct spinlock {
    volatile int locked;
} spinlock_t;

void spin_lock_init(spinlock_t *lock);
void spin_lock(spinlock_t *lock);
void spin_unlock(spinlock_t *lock);

#endif
