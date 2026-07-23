// ThaiOS Thread Manager
// ======================
// Gestione thread: creazione, context switch, terminazione.
// Thread come unità base di schedulazione.

#include <thaios.h>
#include <sched.h>
#include <mm.h>

thread_t *thread_create(process_t *proc, const char *name,
                        void (*entry)(void*), void *arg) {
    thread_t *thread = (thread_t*)kmalloc(sizeof(thread_t));
    if (!thread) return NULL;

    thread->tid = (proc->pid << 16) | (++g_tid_counter & 0xFFFF);
    strncpy(thread->name, name, THREAD_NAME_MAX - 1);
    thread->name[THREAD_NAME_MAX - 1] = '\0';
    thread->state = THREAD_READY;
    thread->priority = DEFAULT_PRIORITY;
    thread->time_slice = TIME_SLICE_MS;
    thread->owner = proc;
    thread->next = NULL;

    // Alloca stack kernel
    thread->kernel_stack = (void*)pmm_alloc_page();
    if (!thread->kernel_stack) {
        kfree(thread);
        return NULL;
    }

    // Configura il contesto iniziale
    memset(&thread->context, 0, sizeof(cpu_context_t));
    thread->context.rsp = (u64)thread->kernel_stack + PAGE_SIZE - 8;
    thread->context.rip = (u64)entry;
    thread->context.cs = 0x08;     // Kernel code segment

    // Set primo argomento
    thread->context.rdi = (u64)arg;

    kprintf("[THREAD] Created thread '%s' (TID: %llu) in process '%s'\n",
            thread->name, thread->tid, proc->name);

    if (!proc->main_thread) {
        proc->main_thread = thread;
    }

    // Aggiungi allo scheduler
    sched_add(thread);

    return thread;
}

void thread_exit(int code) {
    thread_t *current = sched_current();
    if (!current) return;

    kprintf("[THREAD] Thread '%s' (TID: %llu) exiting with code %d\n",
            current->name, current->tid, code);

    current->state = THREAD_ZOMBIE;
    sched_remove(current);

    // TODO: cleanup, wake up joiner

    sched_yield();
}

int thread_join(thread_t *thread) {
    if (!thread) return -1;

    // Attendi che il thread termini
    while (thread->state != THREAD_ZOMBIE && thread->state != THREAD_DEAD) {
        sched_yield();
    }

    // TODO: raccogli codice di uscita

    kfree(thread->kernel_stack);
    kfree(thread);
    return 0;
}

void thread_sleep(u64 ms) {
    thread_t *current = sched_current();
    if (!current) return;

    current->state = THREAD_SLEEPING;
    sched_remove(current);
    // TODO: timer queue per risvegliare dopo ms
    sched_yield();
}
