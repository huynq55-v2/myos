// stacks.h

#ifndef STACKS_H
#define STACKS_H

#include <stdint.h>

extern uint8_t kernel_stack[4096];
extern uint8_t ist1_stack[4096];
extern uint64_t kernel_stack_top;
extern uint64_t ist1_stack_top;

#endif // STACKS_H