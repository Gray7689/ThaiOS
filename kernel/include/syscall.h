#ifndef _SYSCALL_H
#define _SYSCALL_H

#include <thaios.h>

#define SYSCALL_MAX 256

typedef enum syscall_num {
    SYS_exit = 0,
    SYS_fork = 1,
    SYS_read = 2,
    SYS_write = 3,
    SYS_open = 4,
    SYS_close = 5,
    SYS_mmap = 6,
    SYS_munmap = 7,
    SYS_sched_yield = 8,
    SYS_getpid = 9,
    SYS_gettid = 10,
    SYS_sleep = 11,
    SYS_ipc_send = 12,
    SYS_ipc_recv = 13,
    SYS_shm_create = 14,
    SYS_shm_attach = 15,
    SYS_clock_get = 16,
    SYS_ioctl = 17,
    SYS_fs_mount = 18,
    SYS_fs_unmount = 19,
    SYS_net_socket = 20,
    SYS_net_bind = 21,
    SYS_net_listen = 22,
    SYS_net_accept = 23,
    SYS_net_connect = 24,
    SYS_ai_query = 25,
} syscall_num_t;

typedef i64 (*syscall_handler_t)(u64 arg0, u64 arg1, u64 arg2,
                                  u64 arg3, u64 arg4, u64 arg5);

void syscall_init(void);
void syscall_register(u64 num, syscall_handler_t handler);
i64 syscall_dispatch(u64 num, u64 arg0, u64 arg1, u64 arg2,
                     u64 arg3, u64 arg4, u64 arg5);

#endif
