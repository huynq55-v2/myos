#include "spinlock.h"

void spin_lock_init(spinlock_t *lock) {
    lock->locked = 0;
}

void spin_lock(spinlock_t *lock) {
    while(__sync_lock_test_and_set(&(lock->locked), 1)) {
        // Chờ cho đến khi khóa được giải phóng
        while(lock->locked);
    }
}

void spin_unlock(spinlock_t *lock) {
    __sync_lock_release(&(lock->locked));
}
