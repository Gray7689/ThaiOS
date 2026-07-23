// ThaiOS Physical Memory Manager
// ===============================
// Gestione della memoria fisica tramite bitmap.
// Ogni bit rappresenta un frame di 4096 byte.

#include <thaios.h>
#include <mm.h>

#define BITMAP_BITS (PAGE_SIZE * 8)
#define MAX_PAGES (1024 * 1024)    // Fino a 4TB di RAM (1M pagine * 4KB)

static u8 pmm_bitmap[MAX_PAGES / 8];
static usize pmm_total_pages = 0;
static usize pmm_used_pages = 0;
static usize pmm_last_alloc = 0;
static u64 pmm_mem_top = 0;

void pmm_init(u64 memmap_addr, u64 memmap_entries) {
    // Per ora, assumiamo 128MB di RAM
    pmm_total_pages = (128 * 1024 * 1024) / PAGE_SIZE;

    // Marca tutti i frame come allocati (poi li libereremo)
    for (usize i = 0; i < pmm_total_pages / 8; i++) {
        pmm_bitmap[i] = 0xFF;
    }

    // Libera i frame utilizzabili
    for (usize i = 1; i < pmm_total_pages; i++) {
        usize byte_idx = i / 8;
        u8 bit_idx = i % 8;
        pmm_bitmap[byte_idx] &= ~(1 << bit_idx);
    }
    pmm_used_pages = 0;
    pmm_last_alloc = 0;

    // Marca i primi frame (kernel) come usati
    usize kernel_frames = 4096 / PAGE_SIZE;  // ~16MB per kernel
    for (usize i = 0; i < kernel_frames && i < pmm_total_pages; i++) {
        usize byte_idx = i / 8;
        u8 bit_idx = i % 8;
        pmm_bitmap[byte_idx] |= (1 << bit_idx);
        pmm_used_pages++;
    }
}

static bool pmm_is_free(usize page) {
    usize byte_idx = page / 8;
    u8 bit_idx = page % 8;
    return !(pmm_bitmap[byte_idx] & (1 << bit_idx));
}

static void pmm_set_used(usize page) {
    usize byte_idx = page / 8;
    u8 bit_idx = page % 8;
    pmm_bitmap[byte_idx] |= (1 << bit_idx);
}

static void pmm_set_free(usize page) {
    usize byte_idx = page / 8;
    u8 bit_idx = page % 8;
    pmm_bitmap[byte_idx] &= ~(1 << bit_idx);
}

paddr_t pmm_alloc_page(void) {
    if (pmm_used_pages >= pmm_total_pages) {
        panic("Out of physical memory!");
    }

    for (usize i = pmm_last_alloc; i < pmm_total_pages; i++) {
        if (pmm_is_free(i)) {
            pmm_set_used(i);
            pmm_used_pages++;
            pmm_last_alloc = i + 1;
            return (paddr_t)(i * PAGE_SIZE);
        }
    }

    for (usize i = 0; i < pmm_last_alloc; i++) {
        if (pmm_is_free(i)) {
            pmm_set_used(i);
            pmm_used_pages++;
            pmm_last_alloc = i + 1;
            return (paddr_t)(i * PAGE_SIZE);
        }
    }

    panic("Out of physical memory (no free pages)");
    return 0;
}

void pmm_free_page(paddr_t addr) {
    usize page = addr / PAGE_SIZE;
    if (page >= pmm_total_pages) return;
    pmm_set_free(page);
    pmm_used_pages--;
}

usize pmm_free_pages_count(void) {
    return pmm_total_pages - pmm_used_pages;
}
