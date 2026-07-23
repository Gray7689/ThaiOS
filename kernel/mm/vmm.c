// ThaiOS Virtual Memory Manager
// ==============================
// Paging x86_64 a 4 livelli (PML4, PDPT, PD, PT).
// Supporto per mappatura, unmapping e page fault handling.

#include <thaios.h>
#include <mm.h>

// Page table entry flags
#define VMM_PRESENT    (1ULL << 0)
#define VMM_WRITABLE   (1ULL << 1)
#define VMM_USER       (1ULL << 2)
#define VMM_NX         (1ULL << 63)
#define VMM_LARGE      (1ULL << 7)

typedef struct page_table {
    u64 entries[512];
} page_table_t;

static page_table_t *g_pml4 = NULL;

void vmm_init(void) {
    // Leggi PML4 corrente (CR3)
    u64 cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    g_pml4 = (page_table_t*)(cr3 & ~0xFFF);
    kprintf("[VMM] Current PML4 at: %p\n", g_pml4);
}

static page_table_t *vmm_get_or_create_table(page_table_t *parent, usize idx) {
    if (!(parent->entries[idx] & VMM_PRESENT)) {
        paddr_t new_page = pmm_alloc_page();
        if (!new_page) return NULL;

        for (usize i = 0; i < 512; i++) {
            ((page_table_t*)new_page)->entries[i] = 0;
        }

        parent->entries[idx] = new_page | VMM_PRESENT | VMM_WRITABLE;
    }

    return (page_table_t*)(parent->entries[idx] & ~0xFFF);
}

vaddr_t vmm_map_page(paddr_t phys, vaddr_t virt, u64 flags) {
    usize pml4_idx = (virt >> 39) & 0x1FF;
    usize pdpt_idx = (virt >> 30) & 0x1FF;
    usize pd_idx   = (virt >> 21) & 0x1FF;
    usize pt_idx   = (virt >> 12) & 0x1FF;

    page_table_t *pdpt = vmm_get_or_create_table(g_pml4, pml4_idx);
    page_table_t *pd   = vmm_get_or_create_table(pdpt, pdpt_idx);
    page_table_t *pt   = vmm_get_or_create_table(pd, pd_idx);

    if (!pt) return 0;

    pt->entries[pt_idx] = phys | flags | VMM_PRESENT;

    // Invalida TLB per questa pagina
    __asm__ volatile("invlpg %0" : : "m"(*(u8*)virt));

    return virt;
}

void vmm_unmap_page(vaddr_t virt) {
    usize pt_idx = (virt >> 12) & 0x1FF;
    usize pd_idx = (virt >> 21) & 0x1FF;
    usize pdpt_idx = (virt >> 30) & 0x1FF;
    usize pml4_idx = (virt >> 39) & 0x1FF;

    page_table_t *pdpt = (page_table_t*)(g_pml4->entries[pml4_idx] & ~0xFFF);
    if (!pdpt) return;

    page_table_t *pd = (page_table_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);
    if (!pd) return;

    page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);
    if (!pt) return;

    pt->entries[pt_idx] = 0;
    __asm__ volatile("invlpg %0" : : "m"(*(u8*)virt));
}

paddr_t vmm_virt_to_phys(vaddr_t virt) {
    usize pt_idx = (virt >> 12) & 0x1FF;
    usize pd_idx = (virt >> 21) & 0x1FF;
    usize pdpt_idx = (virt >> 30) & 0x1FF;
    usize pml4_idx = (virt >> 39) & 0x1FF;

    page_table_t *pdpt = (page_table_t*)(g_pml4->entries[pml4_idx] & ~0xFFF);
    if (!pdpt) return 0;

    page_table_t *pd = (page_table_t*)(pdpt->entries[pdpt_idx] & ~0xFFF);
    if (!pd) return 0;

    page_table_t *pt = (page_table_t*)(pd->entries[pd_idx] & ~0xFFF);
    if (!pt) return 0;

    u64 entry = pt->entries[pt_idx];
    if (!(entry & VMM_PRESENT)) return 0;

    return (entry & ~0xFFF) | (virt & 0xFFF);
}
