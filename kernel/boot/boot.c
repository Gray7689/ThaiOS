// ThaiOS Boot Initialization
// ===========================
// Setup base del sistema: GDT, IDT, pic, timer.

#include <thaios.h>

extern void _start(void);

// GDT entries
typedef struct gdt_entry {
    u16 limit_low;
    u16 base_low;
    u8  base_middle;
    u8  access;
    u8  granularity;
    u8  base_high;
} __attribute__((packed)) gdt_entry_t;

typedef struct gdt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed)) gdt_ptr_t;

static gdt_entry_t gdt[5];
static gdt_ptr_t gdt_ptr;

void gdt_set_entry(int num, u32 base, u32 limit, u8 access, u8 gran) {
    gdt[num].base_low    = base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;
    gdt[num].limit_low   = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;
    gdt[num].granularity |= gran & 0xF0;
    gdt[num].access      = access;
}

void gdt_init(void) {
    gdt_ptr.limit = sizeof(gdt) - 1;
    gdt_ptr.base  = (u64)&gdt;

    // Null segment
    gdt_set_entry(0, 0, 0, 0, 0);
    // Kernel code
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0xA0);
    // Kernel data
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0xA0);
    // User code
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0xA0);
    // User data
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0xA0);

    __asm__ volatile("lgdt %0" : : "m"(gdt_ptr));
    __asm__ volatile(
        "push $0x08\n"
        "lea .reload_cs(%%rip), %%rax\n"
        "push %%rax\n"
        "retfq\n"
        ".reload_cs:\n"
        "mov $0x10, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        : : : "rax", "memory"
    );
}

// IDT entry
typedef struct idt_entry {
    u16 offset_low;
    u16 selector;
    u8  ist;
    u8  flags;
    u16 offset_mid;
    u32 offset_high;
    u32 zero;
} __attribute__((packed)) idt_entry_t;

typedef struct idt_ptr {
    u16 limit;
    u64 base;
} __attribute__((packed)) idt_ptr_t;

static idt_entry_t idt[256];
static idt_ptr_t idt_ptr;

void idt_set_entry(int num, u64 handler, u16 selector, u8 flags) {
    idt[num].offset_low  = handler & 0xFFFF;
    idt[num].offset_mid  = (handler >> 16) & 0xFFFF;
    idt[num].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[num].selector    = selector;
    idt[num].ist         = 0;
    idt[num].flags       = flags;
    idt[num].zero        = 0;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(idt) - 1;
    idt_ptr.base  = (u64)&idt;

    for (int i = 0; i < 256; i++) {
        idt_set_entry(i, (u64)isr_default_handler, 0x08, 0x8E);
    }

    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}

void isr_default_handler(void) {
    kprintf("[ThaiOS] Unhandled interrupt!\n");
    panic("Unhandled interrupt");
}

void boot_init(void) {
    kprintf("[ThaiOS] Setting up GDT...\n");
    gdt_init();

    kprintf("[ThaiOS] Setting up IDT...\n");
    idt_init();
}
