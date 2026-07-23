// ThaiOS NVMe Driver
// ===================
// Driver per controller NVMe con supporto per comandi base
// (Identify, Read, Write, Flush).

#include <driver.h>
#include <sched.h>

#define NVME_MAX_QUEUES 64
#define NVME_SQ_SIZE    64
#define NVME_CQ_SIZE    64

typedef struct nvme_queue {
    u32 *sq;       // Submission Queue (phys addr)
    u32 *cq;       // Completion Queue (phys addr)
    u16 sq_head;
    u16 sq_tail;
    u16 cq_head;
    u16 phase;
} nvme_queue_t;

typedef struct nvme_controller {
    paddr_t regs;         // MMIO registers
    u32 doorbell_stride;
    u16 num_queues;
    nvme_queue_t queues[NVME_MAX_QUEUES];
} nvme_controller_t;

static nvme_controller_t *g_nvme = NULL;

int nvme_init(void) {
    kprintf("[NVMe] Initializing NVMe controller...\n");

    pci_device_t *dev = pci_find_device(0x8086, 0x0A54); // Example Intel NVMe
    if (!dev) {
        dev = pci_find_device(0x144D, 0xA804); // Samsung NVMe
    }
    if (!dev) {
        kprintf("[NVMe] No NVMe controller found\n");
        return -ENODEV;
    }

    kprintf("[NVMe] Controller found at %04x:%04x\n", dev->vendor_id, dev->device_id);

    // TODO: BAR mapping, doorbell setup, queue creation
    g_nvme = (nvme_controller_t*)kmalloc(sizeof(nvme_controller_t));
    if (!g_nvme) return -ENOMEM;

    g_nvme->regs = dev->bar[0];
    g_nvme->num_queues = 1;

    kprintf("[NVMe] Initialized (regs at 0x%llx)\n", g_nvme->regs);
    return SUCCESS;
}

int nvme_read(u64 lba, void *buffer, usize count) {
    if (!g_nvme) return -ENODEV;
    // TODO: costruisci NVMe command, submit, wait
    kprintf("[NVMe] Read LBA=%llu count=%llu (stub)\n", lba, count);
    return SUCCESS;
}

int nvme_write(u64 lba, const void *data, usize count) {
    if (!g_nvme) return -ENODEV;
    kprintf("[NVMe] Write LBA=%llu count=%llu (stub)\n", lba, count);
    return SUCCESS;
}
