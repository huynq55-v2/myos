void _start() {
    while (1) {
        // Optionally, you can add a pause or other non-privileged instructions here
        asm volatile("nop"); // No Operation - does nothing, safe in user mode
    }
}
