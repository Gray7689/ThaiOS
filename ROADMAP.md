# ThaiOS Roadmap

## Panoramica

ThaiOS segue un processo di sviluppo incrementale con 10 milestone (M0-M10).
Ogni milestone produce un sistema avviabile e testabile.

---

## M0 — Boot (4-6 settimane)

- [x] Repository structure
- [x] Build system (Makefile + CMake)
- [ ] Bootloader configuration (Limine)
- [ ] Entry point assembly (x86_64 long mode)
- [ ] Serial output (early debug)
- [ ] GDT/IDT setup
- [ ] Basic paging
- [ ] Hello world da kernel space

**Risultato:** ThaiOS avvia e stampa messaggi su seriale

---

## M1 — Kernel base (8-12 settimane)

- [ ] Physical Memory Manager (bitmap)
- [ ] Virtual Memory Manager (4-level paging)
- [ ] Heap kernel (slab + buddy)
- [ ] Scheduler O(1) con SMP support
- [ ] Process/Thread management
- [ ] IPC (message passing + shared memory)
- [ ] System call table
- [ ] Timer (HPET/APIC)

**Risultato:** Kernel multitasking con processi e IPC

---

## M2 — Drivers (8-10 settimane)

- [ ] PCI/PCIe enumeration
- [ ] ATA/SATA/NVMe storage driver
- [ ] PS/2 + USB HID input
- [ ] Framebuffer (VESA BIOS Extensions)
- [ ] AHCI driver
- [ ] Interrupt handling (IOAPIC + MSI)

**Risultato:** ThaiOS rileva hardware e carica driver

---

## M3 — File System (6-8 settimane)

- [ ] VFS layer
- [ ] ext4 read-only
- [ ] FAT32 read/write
- [ ] ThaiFS design completion
- [ ] ThaiFS basic operations
- [ ] Initramfs

**Risultato:** ThaiOS monta filesystem e legge file

---

## M4 — Networking (8-12 settimane)

- [ ] Network Interface Card driver (e1000, rtl8139)
- [ ] IPv4 + ARP
- [ ] ICMP (ping funzionante)
- [ ] UDP
- [ ] TCP (3-way handshake)
- [ ] BSD socket API
- [ ] DHCP client
- [ ] DNS resolver

**Risultato:** ThaiOS ha connettività di rete base

---

## M5 — Userland (6-8 settimane)

- [ ] libc portabile (musl-based)
- [ ] ELF loader
- [ ] ThaiShell (command-line base)
- [ ] Init process
- [ ] System calls complete
- [ ] Signal handling

**Risultato:** Shell interattiva con comandi base

---

## M6 — GUI (10-14 settimane)

- [ ] Framebuffer console
- [ ] Compositor (simple)
- [ ] Wayland protocol (subset)
- [ ] Window manager (Wiri WM)
- [ ] Dock + Taskbar
- [ ] Start menu
- [ ] Notification center
- [ ] Themes (light + dark)
- [ ] Font rendering (stb_truetype)

**Risultato:** Desktop funzionante con mouse e finestre

---

## M7 — Applications (8-12 settimane)

- [ ] ThaiTerminal (VT100-compatible)
- [ ] ThaiFiles (file manager)
- [ ] ThaiSettings (control panel)
- [ ] ThaiViewer (image viewer)
- [ ] ThaiCalc (calculator)
- [ ] ThaiEditor (text editor)
- [ ] ThaiMonitor (system monitor)
- [ ] ThaiBrowser (webkit-based, minimal)

**Risultato:** Sistema usabile per task quotidiani

---

## M8 — Security (6-8 settimane)

- [ ] Sandbox (namespace + seccomp)
- [ ] Firewall (netfilter)
- [ ] Disk encryption (LUKS-like)
- [ ] Secure Boot
- [ ] Signed updates
- [ ] Permission system

**Risultato:** Sistema sicuro per uso quotidiano

---

## M9 — ThaiAI (8-10 settimane)

- [ ] Local LLM integration (llama.cpp)
- [ ] Voice recognition (whisper.cpp)
- [ ] System control API
- [ ] File search
- [ ] Automation engine
- [ ] Code generation

**Risultato:** Assistente AI locale integrato

---

## M10 — Release (ongoing)

- [ ] Performance optimization
- [ ] Hardware compatibility
- [ ] Bug fixes
- [ ] Documentation
- [ ] Community building
- [ ] Package repository

**Risultato:** ThaiOS v1.0.0-beta

---

## Contribuire

Vedi [CONTRIBUTING.md](CONTRIBUTING.md) per linee guida.
