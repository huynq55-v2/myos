// stacks.c

#include "stacks.h"

uint8_t kernel_stack[4096];
uint8_t ist1_stack[4096];

uint64_t kernel_stack_top = (uint64_t)(kernel_stack + sizeof(kernel_stack));
uint64_t ist1_stack_top = (uint64_t)(ist1_stack + sizeof(ist1_stack));