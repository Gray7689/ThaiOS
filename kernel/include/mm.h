#ifndef _MM_H
#define _MM_H

#include <thaios.h>

typedef struct page {
    struct page *next;
    u64 flags;
} page_t;

#define PAGE_FLAG_ALLOCATED (1ULL << 0)
#define PAGE_FLAG_KERNEL    (1ULL << 1)
#define PAGE_FLAG_RESERVED  (1ULL << 2)

void pmm_init(u64 memmap_addr, u64 memmap_entries);
paddr_t pmm_alloc_page(void);
void pmm_free_page(paddr_t addr);
usize pmm_free_pages_count(void);

void vmm_init(void);
vaddr_t vmm_map_page(paddr_t phys, vaddr_t virt, u64 flags);
void vmm_unmap_page(vaddr_t virt);
paddr_t vmm_virt_to_phys(vaddr_t virt);

void heap_init(vaddr_t start, usize size);
void *kmalloc(usize size);
void kfree(void *ptr);
void *kcalloc(usize num, usize size);
void *krealloc(void *ptr, usize new_size);

#endif
