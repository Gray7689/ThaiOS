// ThaiOS System Call Dispatcher
// ==============================
// Dispatch delle system call dalla tabella vettorizzata.
// Fast-path per syscall semplici, slow-path per syscall complesse.

#include <thaios.h>
#include <syscall.h>
#include <sched.h>
#include <mm.h>

extern void syscall_entry(void);  // Assembly handler

static syscall_handler_t g_syscall_table[SYSCALL_MAX];

static i64 sys_exit_handler(u64 code, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);
static i64 sys_getpid_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);
static i64 sys_write_handler(u64 fd, u64 buf, u64 count, u64 a3, u64 a4, u64 a5);
static i64 sys_sched_yield_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);
static i64 sys_clock_get_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5);

void syscall_init(void) {
    for (u64 i = 0; i < SYSCALL_MAX; i++) {
        g_syscall_table[i] = NULL;
    }

    // Registra i syscall handler
    syscall_register(SYS_exit,        sys_exit_handler);
    syscall_register(SYS_getpid,      sys_getpid_handler);
    syscall_register(SYS_write,       sys_write_handler);
    syscall_register(SYS_sched_yield, sys_sched_yield_handler);
    syscall_register(SYS_clock_get,   sys_clock_get_handler);

    kprintf("[SYSCALL] System call table initialized (%d slots)\n", SYSCALL_MAX);

    // Configura MSR per syscall/sysret (x86_64)
    u64 star = ((u64)0x08 << 32) | ((u64)0x10 << 48);
    u64 lstar = (u64)syscall_entry;
    u64 cstar = (u64)syscall_entry;
    u64 sfmask = 0x300;  // Clear IF and DF on syscall entry

    __asm__ volatile(
        "wrmsr\n"
        : : "c"(0xC0000081), "a"(star & 0xFFFFFFFF), "d"(star >> 32)
    );
    __asm__ volatile(
        "wrmsr\n"
        : : "c"(0xC0000082), "a"(lstar & 0xFFFFFFFF), "d"(lstar >> 32)
    );
    __asm__ volatile(
        "wrmsr\n"
        : : "c"(0xC0000084), "a"(sfmask & 0xFFFFFFFF), "d"(sfmask >> 32)
    );
}

void syscall_register(u64 num, syscall_handler_t handler) {
    if (num < SYSCALL_MAX) {
        g_syscall_table[num] = handler;
    }
}

i64 syscall_dispatch(u64 num, u64 arg0, u64 arg1, u64 arg2,
                     u64 arg3, u64 arg4, u64 arg5) {
    if (num >= SYSCALL_MAX || !g_syscall_table[num]) {
        kprintf("[SYSCALL] Unknown syscall %llu\n", num);
        return -ENOTSUP;
    }

    return g_syscall_table[num](arg0, arg1, arg2, arg3, arg4, arg5);
}

// === Handler implementations ===

static i64 sys_exit_handler(u64 code, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5) {
    thread_exit((int)code);
    return 0;
}

static i64 sys_getpid_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5) {
    thread_t *current = sched_current();
    if (current && current->owner) return current->owner->pid;
    return -1;
}

static i64 sys_write_handler(u64 fd, u64 buf, u64 count, u64 a3, u64 a4, u64 a5) {
    if (fd == 1 || fd == 2) {
        char *str = (char*)buf;
        for (u64 i = 0; i < count && str[i]; i++) {
            kprintf("%c", str[i]);
        }
        return count;
    }
    return -EINVAL;
}

static i64 sys_sched_yield_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5) {
    sched_yield();
    return 0;
}

static i64 sys_clock_get_handler(u64 a0, u64 a1, u64 a2, u64 a3, u64 a4, u64 a5) {
    extern u64 get_system_ticks(void);
    return get_system_ticks();
}
