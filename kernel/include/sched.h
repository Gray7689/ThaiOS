#ifndef _SCHED_H
#define _SCHED_H

#include <thaios.h>

#define THREAD_NAME_MAX 64
#define MAX_PRIORITY    139
#define MIN_PRIORITY    0
#define DEFAULT_PRIORITY 60

typedef enum thread_state {
    THREAD_READY,
    THREAD_RUNNING,
    THREAD_BLOCKED,
    THREAD_SLEEPING,
    THREAD_ZOMBIE,
    THREAD_DEAD
} thread_state_t;

typedef struct cpu_context {
    u64 r15, r14, r13, r12, r11, r10, r9, r8;
    u64 rbp, rdi, rsi, rdx, rcx, rbx, rax;
    u64 rip, cs, rflags, rsp, ss;
} cpu_context_t;

typedef struct thread {
    u64 tid;
    char name[THREAD_NAME_MAX];
    cpu_context_t context;
    thread_state_t state;
    u64 priority;
    u64 time_slice;
    struct process *owner;
    struct thread *next;
    void *kernel_stack;
    void *user_stack;
} thread_t;

typedef struct process {
    u64 pid;
    char name[THREAD_NAME_MAX];
    vaddr_t address_space;
    struct thread *main_thread;
    struct process *next;
    u64 state;
    vaddr_t heap_start;
    vaddr_t heap_end;
} process_t;

void sched_init(void);
void sched_add(thread_t *thread);
void sched_remove(thread_t *thread);
void sched_yield(void);
thread_t *sched_current(void);
void sched_tick(void);

process_t *proc_create(const char *name);
void proc_destroy(process_t *proc);
thread_t *thread_create(process_t *proc, const char *name,
                        void (*entry)(void*), void *arg);
void thread_exit(int code);
int thread_join(thread_t *thread);

#endif
