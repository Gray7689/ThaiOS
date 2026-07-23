// ThaiOS Process Manager
// =======================
// Gestione processi: creazione, distruzione, PCB.
// Process Control Block minimale con address space separato.

#include <thaios.h>
#include <sched.h>
#include <mm.h>

static process_t *g_process_list = NULL;
static usize g_process_count = 0;

process_t *proc_create(const char *name) {
    process_t *proc = (process_t*)kmalloc(sizeof(process_t));
    if (!proc) return NULL;

    proc->pid = g_process_count++;
    strncpy(proc->name, name, THREAD_NAME_MAX - 1);
    proc->name[THREAD_NAME_MAX - 1] = '\0';
    proc->main_thread = NULL;
    proc->next = NULL;
    proc->state = 0;

    // Alloca address space (PML4 per ora usa quello kernel)
    proc->address_space = 0;  // TODO: fork/exec con address space separato

    // Heap virtuale iniziale
    proc->heap_start = 0x600000000000;
    proc->heap_end = proc->heap_start;

    kprintf("[PROC] Created process '%s' (PID: %llu)\n", proc->name, proc->pid);

    // Aggiungi alla lista globale
    proc->next = g_process_list;
    g_process_list = proc;

    return proc;
}

void proc_destroy(process_t *proc) {
    if (!proc) return;

    kprintf("[PROC] Destroying process '%s' (PID: %llu)\n", proc->name, proc->pid);

    // TODO: kill all threads, free memory, etc.

    // Rimuovi dalla lista
    if (g_process_list == proc) {
        g_process_list = proc->next;
    } else {
        process_t *prev = g_process_list;
        while (prev && prev->next != proc) prev = prev->next;
        if (prev) prev->next = proc->next;
    }

    kfree(proc);
}

process_t *proc_by_pid(u64 pid) {
    process_t *proc = g_process_list;
    while (proc) {
        if (proc->pid == pid) return proc;
        proc = proc->next;
    }
    return NULL;
}
