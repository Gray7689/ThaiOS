// ThaiOS IPC - Shared Memory
// ============================
// Condivisione di memoria tra processi con ring buffer lock-free.

#include <thaios.h>
#include <ipc.h>
#include <mm.h>

#define MAX_SHM_SEGMENTS 128

static shared_mem_t g_shm_segments[MAX_SHM_SEGMENTS];
static usize g_shm_count = 0;

int shm_create(usize size, u64 flags) {
    if (g_shm_count >= MAX_SHM_SEGMENTS) return -ENOMEM;

    size = PAGE_ALIGN(size);
    int shm_id = -1;

    for (usize i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (g_shm_segments[i].id == 0 && g_shm_segments[i].refcount == 0) {
            shm_id = i;
            break;
        }
    }

    if (shm_id < 0) return -ENOMEM;

    // Alloca pagine fisiche
    usize num_pages = size / PAGE_SIZE;
    for (usize i = 0; i < num_pages; i++) {
        paddr_t page = pmm_alloc_page();
        if (!page) {
            // Rollback
            for (usize j = 0; j < i; j++) {
                pmm_free_page(g_shm_segments[shm_id].addr + j * PAGE_SIZE);
            }
            return -ENOMEM;
        }
    }

    g_shm_segments[shm_id].id = shm_id + 1;
    g_shm_segments[shm_id].addr = 0;  // Assegnato in attach
    g_shm_segments[shm_id].size = size;
    g_shm_segments[shm_id].flags = flags;
    g_shm_segments[shm_id].refcount = 0;
    g_shm_count++;

    kprintf("[SHM] Created segment %d, size %llu bytes\n", shm_id, size);
    return shm_id;
}

int shm_attach(int shm_id, vaddr_t *addr) {
    if (shm_id < 0 || shm_id >= MAX_SHM_SEGMENTS) return -EINVAL;
    if (g_shm_segments[shm_id].id == 0) return -EINVAL;

    shared_mem_t *seg = &g_shm_segments[shm_id];

    // Alloca spazio virtuale per mappatura
    vaddr_t virt = 0x7F0000000000 + (shm_id * 0x100000000);
    usize num_pages = seg->size / PAGE_SIZE;

    // TODO: mappa le pagine fisiche nel virtuale
    // per ora mappiamo solo contiguamente
    for (usize i = 0; i < num_pages; i++) {
        vmm_map_page(seg->addr ? 0 : i * PAGE_SIZE,
                     virt + i * PAGE_SIZE,
                     VMM_USER | VMM_WRITABLE);
    }

    seg->addr = virt;
    seg->refcount++;
    *addr = virt;

    return SUCCESS;
}

int shm_detach(vaddr_t addr) {
    for (usize i = 0; i < MAX_SHM_SEGMENTS; i++) {
        if (g_shm_segments[i].addr == addr) {
            g_shm_segments[i].refcount--;
            if (g_shm_segments[i].refcount == 0) {
                g_shm_segments[i].id = 0;
            }
            return SUCCESS;
        }
    }
    return -EINVAL;
}

int shm_destroy(int shm_id) {
    if (shm_id < 0 || shm_id >= MAX_SHM_SEGMENTS) return -EINVAL;
    if (g_shm_segments[shm_id].id == 0) return -EINVAL;

    g_shm_segments[shm_id].id = 0;
    g_shm_segments[shm_id].refcount = 0;
    g_shm_count--;

    return SUCCESS;
}
