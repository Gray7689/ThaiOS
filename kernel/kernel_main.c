// ThaiOS Kernel Main
// ===================
// Entry point C function, chiamata da entry.asm dopo setup base.

#include <thaios.h>
#include <mm.h>
#include <sched.h>
#include <ipc.h>
#include <syscall.h>

// Struttura Limine boot info semplificata
typedef struct limine_boot_info {
    u64 revision;
    u64 reserved[5];
    // ... campi specifici Limine
} limine_boot_info_t;

static limine_boot_info_t *g_boot_info = NULL;
static u64 g_ticks = 0;

void timer_handler(void) {
    g_ticks++;
    sched_tick();
}

u64 get_system_ticks(void) {
    return g_ticks;
}

void kmain(limine_boot_info_t *boot_info) {
    g_boot_info = boot_info;

    // 1. Initializzazione base
    kprintf("[ThaiOS] ThaiOS v%d.%d.%d booting...\n",
            THAIOS_VERSION_MAJOR, THAIOS_VERSION_MINOR, THAIOS_VERSION_PATCH);
    kprintf("[ThaiOS] Architecture: x86_64\n");
    kprintf("[ThaiOS] Boot info at: %p\n", boot_info);

    // 2. Memory manager
    kprintf("[ThaiOS] Initializing PMM...\n");
    pmm_init(0, 0);     // TODO: pass memmap da boot_info
    kprintf("[ThaiOS] Free memory: %llu pages (%llu MB)\n",
            pmm_free_pages_count(), pmm_free_pages_count() * 4 / 1024);

    kprintf("[ThaiOS] Initializing VMM...\n");
    vmm_init();

    kprintf("[ThaiOS] Initializing heap...\n");
    heap_init((vaddr_t)0xFFFF800000000000, 64 * 1024 * 1024);

    // 3. Scheduler
    kprintf("[ThaiOS] Initializing scheduler...\n");
    sched_init();

    // 4. IPC
    kprintf("[ThaiOS] Initializing IPC...\n");
    ipc_init();

    // 5. System calls
    kprintf("[ThaiOS] Initializing syscalls...\n");
    syscall_init();

    // 6. Timer (APIC timer o HPET)
    kprintf("[ThaiOS] Configuring timer...\n");
    // TODO: init_timer(timer_handler);

    // 7. Creazione processo init
    kprintf("[ThaiOS] Starting init process...\n");
    process_t *init = proc_create("init");
    thread_create(init, "init_main", (void (*)(void*))kdemo, NULL);

    // 8. Abilita interrupts e avvia scheduler
    kprintf("[ThaiOS] System ready! Enabling interrupts...\n");
    __asm__ volatile("sti");

    // 9. Loop idle
    while (1) {
        __asm__ volatile("hlt");
    }
}

// Demo thread per mostrare che il sistema funziona
void kdemo(void) {
    int counter = 0;
    while (1) {
        kprintf("[ThaiOS] System alive! Tick=%llu, counter=%d\n",
                get_system_ticks(), counter++);
        for (volatile u64 i = 0; i < 50000000; i++);
    }
}
