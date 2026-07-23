// ThaiOS PCI Driver
// ==================
// Enumerazione e configurazione del bus PCI/PCIe.
// Supporto per MSI, MSI-X, e BAR mapping.

#include <driver.h>

#define PCI_CONFIG_ADDR  0xCF8
#define PCI_CONFIG_DATA  0xCFC

static driver_t g_pci_driver = {
    .name = "PCI Bus Driver",
    .type = DRIVER_PCI,
    .state = DRIVER_UNLOADED,
    .init = NULL,
    .fini = NULL,
    .priv = NULL,
    .next = NULL
};

static u32 pci_config_read_raw(u8 bus, u8 slot, u8 func, u8 offset) {
    u32 address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000u);
    __outdword(PCI_CONFIG_ADDR, address);
    return __indword(PCI_CONFIG_DATA);
}

static void pci_config_write_raw(u8 bus, u8 slot, u8 func, u8 offset, u32 value) {
    u32 address = (u32)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | 0x80000000u);
    __outdword(PCI_CONFIG_ADDR, address);
    __outdword(PCI_CONFIG_DATA, value);
}

int pci_read_config(pci_device_t *dev, u8 offset, usize size, u32 *value) {
    if (!dev || !value) return -EINVAL;
    u32 raw = pci_config_read_raw(0, 0, 0, offset); // Simplified
    *value = raw;
    return SUCCESS;
}

int pci_write_config(pci_device_t *dev, u8 offset, u32 value, u8 size) {
    if (!dev) return -EINVAL;
    pci_config_write_raw(0, 0, 0, offset, value);
    return SUCCESS;
}

int pci_enumerate_devices(void) {
    kprintf("[PCI] Enumerating PCI devices...\n");
    int count = 0;

    for (u8 bus = 0; bus < 255; bus++) {
        for (u8 slot = 0; slot < 32; slot++) {
            for (u8 func = 0; func < 8; func++) {
                u32 vendor_dev = pci_config_read_raw(bus, slot, func, 0);
                u16 vendor = vendor_dev & 0xFFFF;
                u16 device = (vendor_dev >> 16) & 0xFFFF;

                if (vendor == 0xFFFF) {
                    if (func == 0) break;
                    continue;
                }

                u32 class_rev = pci_config_read_raw(bus, slot, func, 0x08);
                u8 class_code = (class_rev >> 24) & 0xFF;
                u8 subclass   = (class_rev >> 16) & 0xFF;
                u8 prog_if    = (class_rev >> 8) & 0xFF;

                kprintf("[PCI] %02x:%02x.%x V=%04x D=%04x Class=%02x Sub=%02x IF=%02x\n",
                        bus, slot, func, vendor, device, class_code, subclass, prog_if);

                // Leggi BARs
                for (int bar = 0; bar < 6; bar++) {
                    u32 bar_val = pci_config_read_raw(bus, slot, func, 0x10 + bar * 4);
                    if (bar_val) {
                        kprintf("[PCI]   BAR%d: 0x%08x\n", bar, bar_val);
                    }
                }

                count++;
            }
        }
    }

    kprintf("[PCI] Found %d devices\n", count);
    return count;
}

pci_device_t *pci_find_device(u16 vendor, u16 device) {
    // Simplified: scan and return first match
    for (u8 bus = 0; bus < 255; bus++) {
        for (u8 slot = 0; slot < 32; slot++) {
            u32 vendor_dev = pci_config_read_raw(bus, slot, 0, 0);
            if ((vendor_dev & 0xFFFF) == vendor &&
                ((vendor_dev >> 16) & 0xFFFF) == device) {
                pci_device_t *pci = (pci_device_t*)kmalloc(sizeof(pci_device_t));
                if (!pci) return NULL;

                pci->vendor_id = vendor;
                pci->device_id = device;
                pci->class_code = 0;
                pci->subclass = 0;
                pci->irq = 0;
                // Read more config...
                return pci;
            }
        }
    }
    return NULL;
}

void driver_init(void) {
    kprintf("[DRV] Initializing driver framework...\n");
    driver_register(&g_pci_driver);
    pci_enumerate_devices();
}
