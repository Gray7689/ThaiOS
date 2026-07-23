// ThaiOS Kernel Heap Allocator
// ==============================
// Slab allocator per piccole allocazioni, Buddy allocator per pagine.
// Implementazione simple heap per bootstrap.

#include <thaios.h>
#include <mm.h>

#define HEAP_MAGIC 0xDEADBEEF
#define HEAP_MIN_SIZE 32
#define HEAP_ALIGN 16

typedef struct heap_block {
    u32 magic;
    usize size;
    bool free;
    struct heap_block *next;
    struct heap_block *prev;
} heap_block_t;

static vaddr_t heap_start = 0;
static vaddr_t heap_end = 0;
static heap_block_t *heap_first = NULL;

void heap_init(vaddr_t start, usize size) {
    heap_start = start;
    heap_end = start + size;

    heap_first = (heap_block_t*)start;
    heap_first->magic = HEAP_MAGIC;
    heap_first->size = size - sizeof(heap_block_t);
    heap_first->free = true;
    heap_first->next = NULL;
    heap_first->prev = NULL;

    kprintf("[HEAP] Initialized at %p, size %llu KB\n", start, size / 1024);
}

void *kmalloc(usize size) {
    if (size == 0) return NULL;
    if (size < HEAP_MIN_SIZE) size = HEAP_MIN_SIZE;

    size = ALIGN_UP(size, HEAP_ALIGN);

    heap_block_t *block = heap_first;
    while (block) {
        if (block->magic != HEAP_MAGIC) panic("Heap corruption detected!");

        if (block->free && block->size >= size) {
            if (block->size > size + sizeof(heap_block_t) + HEAP_MIN_SIZE) {
                heap_block_t *new_block = (heap_block_t*)((u8*)block + sizeof(heap_block_t) + size);
                new_block->magic = HEAP_MAGIC;
                new_block->size = block->size - size - sizeof(heap_block_t);
                new_block->free = true;
                new_block->next = block->next;
                new_block->prev = block;

                if (block->next) block->next->prev = new_block;
                block->next = new_block;
                block->size = size;
            }

            block->free = false;
            memset((u8*)block + sizeof(heap_block_t), 0, size);
            return (void*)((u8*)block + sizeof(heap_block_t));
        }

        block = block->next;
    }

    // Se arriviamo qui, abbiamo esaurito la memoria heap
    kprintf("[HEAP] Out of memory! Requested %llu bytes\n", size);
    return NULL;
}

void kfree(void *ptr) {
    if (!ptr) return;

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    if (block->magic != HEAP_MAGIC) {
        panic("kfree: invalid pointer or corrupted heap");
    }

    block->free = true;

    // Merge con blocchi adiacenti liberi
    if (block->next && block->next->free) {
        block->size += sizeof(heap_block_t) + block->next->size;
        block->next = block->next->next;
        if (block->next) block->next->prev = block;
    }

    if (block->prev && block->prev->free) {
        block->prev->size += sizeof(heap_block_t) + block->size;
        block->prev->next = block->next;
        if (block->next) block->next->prev = block->prev;
    }
}

void *kcalloc(usize num, usize size) {
    usize total = num * size;
    void *ptr = kmalloc(total);
    if (ptr) memset(ptr, 0, total);
    return ptr;
}

void *krealloc(void *ptr, usize new_size) {
    if (!ptr) return kmalloc(new_size);
    if (new_size == 0) { kfree(ptr); return NULL; }

    heap_block_t *block = (heap_block_t*)((u8*)ptr - sizeof(heap_block_t));
    usize old_size = block->size;

    void *new_ptr = kmalloc(new_size);
    if (new_ptr) {
        memcpy(new_ptr, ptr, old_size < new_size ? old_size : new_size);
        kfree(ptr);
    }
    return new_ptr;
}
