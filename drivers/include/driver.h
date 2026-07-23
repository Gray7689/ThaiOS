// ThaiOS Driver Framework Header
// ================================
// Interfaccia unificata per tutti i driver del sistema.

#ifndef _DRIVER_H
#define _DRIVER_H

#include <thaios.h>

#define DRIVER_NAME_MAX 64
#define DRIVER_MAX 256

typedef enum driver_type {
    DRIVER_STORAGE,
    DRIVER_NETWORK,
    DRIVER_INPUT,
    DRIVER_VIDEO,
    DRIVER_AUDIO,
    DRIVER_USB,
    DRIVER_PCI,
    DRIVER_OTHER
} driver_type_t;

typedef enum driver_state {
    DRIVER_UNLOADED,
    DRIVER_LOADED,
    DRIVER_INITIALIZED,
    DRIVER_ERROR
} driver_state_t;

typedef struct driver {
    char name[DRIVER_NAME_MAX];
    driver_type_t type;
    driver_state_t state;
    int (*init)(struct driver *self);
    int (*fini)(struct driver *self);
    void *priv;                     // Dati privati del driver
    struct driver *next;
} driver_t;

typedef struct pci_device {
    u16 vendor_id;
    u16 device_id;
    u16 command;
    u16 status;
    u8  revision_id;
    u8  prog_if;
    u8  subclass;
    u8  class_code;
    u8  multi_func;
    u8  header_type;
    u32 bar[6];
    u8  irq;
} pci_device_t;

// Driver manager API
void driver_init(void);
int driver_register(driver_t *drv);
int driver_unregister(driver_t *drv);
driver_t *driver_find(const char *name);
void driver_enumerate_all(void);

// PCI operations
int pci_enumerate_devices(void);
int pci_read_config(pci_device_t *dev, u8 offset, usize size, u32 *value);
int pci_write_config(pci_device_t *dev, u8 offset, u32 value, u8 size);
pci_device_t *pci_find_device(u16 vendor, u16 device);

// DMA
typedef struct dma_region {
    paddr_t phys;
    vaddr_t virt;
    usize   size;
} dma_region_t;

dma_region_t *dma_alloc(usize size);
void dma_free(dma_region_t *region);

#endif
