# ThaiOS Architecture

## Scelta architetturale: Microkernel ibrido (Modalità 1)

### Motivazione

Abbiamo scelto un **microkernel ibrido** per ThaiOS. Questa decisione è fondata su un'analisi
approfondita dei trade-off tra kernel monolitici (Linux) e microkernel puri (Minix, seL4).

### Vantaggi del microkernel ibrido

| Aspetto | Vantaggio |
|---------|-----------|
| **Sicurezza** | I driver e i servizi di sistema girano in user-space, isolati tra loro |
| **Stabilità** | Un crash in un driver non abbatte l'intero sistema |
| **Manutenibilità** | Codice modulare, ogni componente è indipendente |
| **Portabilità** | Il kernel minimo è facile da portare su nuove architetture |
| **Performance** | I servizi critici (scheduler, IPC) restano in kernel-space per evitare overhead |

### Svantaggi (e mitigazioni)

| Svantaggio | Mitigazione |
|------------|-------------|
| IPC overhead maggiore | IPC ottimizzato con shared memory e messaggi leggeri |
| Complessità di design | Architettura layer ben definita con API chiare |
| Maturità | Community-driven, roadmap a lungo termine |

### Confronto: Modalità 2 (basato su Linux) sarebbe...

...più rapido da sviluppare ma limiterebbe la nostra libertà architetturale.
ThaiOS mira a essere un sistema *nuovo*, non una distribuzione Linux.

---

## Diagramma architetturale

```
                    ┌─────────────────────────────────────┐
                    │          User Applications          │
                    │   (ThaiShell, ThaiFiles, Browser)   │
                    └────────────────┬────────────────────┘
                                     │ syscall
                    ┌────────────────▼────────────────────┐
                    │         System Libraries (libc)      │
                    └────────────────┬────────────────────┘
                                     │
                    ┌────────────────▼────────────────────┐
                    │          Server di Sistema           │
                    │  (File Server, Net Server, GUI Srv)  │
                    └────────────────┬────────────────────┘
                                     │ IPC
                    ┌────────────────▼────────────────────┐
                    │      Microkernel (kernel space)      │
                    │  ┌──────┬──────┬──────┬──────────┐   │
                    │  │ Sched│ MM   │ IPC  │ Syscall  │   │
                    │  └──────┴──────┴──────┴──────────┘   │
                    │  ┌──────┬──────┬──────┬──────────┐   │
                    │  │ Boot │ Int  │ Timer│ Security │   │
                    │  └──────┴──────┴──────┴──────────┘   │
                    └────────────────┬────────────────────┘
                                     │
                    ┌────────────────▼────────────────────┐
                    │         Hardware Abstraction         │
                    │         (x86, ARM, RISC-V)           │
                    └─────────────────────────────────────┘
```

---

## Componenti del kernel

### 1. Boot & Initialization
Il bootloader (Limine) carica il kernel in modalità long mode (x86_64) o con
paging attivo (ARM). Il kernel inizializza:
- GDT/IDT (x86) o equivalenti (ARM)
- Tabella delle pagine
- BSP (Boot Strap Processor)
- System call handlers

### 2. Memory Manager (MM)
**Physical Memory Manager (PMM)**
- Bitmap per tracciare i frame fisici
- Allocazione contigua per DMA
- Supporto NUMA

**Virtual Memory Manager (VMM)**
- Paging a 4 livelli (x86_64) o equivalenti
- Copy-on-Write per fork
- Mappatura lazy
- Swap su disco

**Heap kernel**
- Slab allocator per strutture piccole
- Buddy allocator per pagine

### 3. Scheduler
- Scheduling O(1) con code multiple
- Supporto SMP (affinità CPU)
- Priorità dinamiche
- CFS (Completely Fair Scheduler) per processi interattivi
- Budget temporale configurabile (default 4ms)

### 4. Process & Thread Manager
- Process Control Block (PCB) minimo
- Thread come unità di schedulazione
- Processi leggeri (fork con COW)
- Signal handling POSIX-compatibile

### 5. IPC (Inter-Process Communication)
- Message passing ottimizzato
- Shared memory con ring buffer lock-free
- Semafori e mutex priority-inheritance
- Eventi e notifiche asincrone

### 6. System Call Interface
- Tabella syscall vettorizzata
- Fast path per syscall semplici (getpid, clock_gettime)
- Argomenti passati via registri (zero-copy)

---

## File System (VFS + ThaiFS)

```
                    ┌──────────────────────────────┐
                    │        System calls          │
                    │   (open, read, write, ioctl) │
                    └──────────┬───────────────────┘
                               │
                    ┌──────────▼───────────────────┐
                    │   Virtual File System (VFS)  │
                    │  ┌────┐ ┌────┐ ┌────┐ ┌────┐│
                    │  │ext4│ │btrf│ │NTFS│ │Thai││
                    │  └────┘ └────┘ └────┘ └────┘│
                    └──────────┬───────────────────┘
                               │
                    ┌──────────▼───────────────────┐
                    │      Block Device Layer       │
                    └──────────────────────────────┘
```

**ThaiFS** — file system proprietario con:
- B-tree ottimizzato per SSDs
- Checksum su ogni metadata block
- Compressione trasparente (ZSTD)
- Deduplicazione inline
- Snapshot atomici
- Cifratura nativa (AES-256-XTS)

---

## Networking Stack

Stack di rete completo con architettura a layer:

```
┌──────────────────────────────────────┐
│        Socket API (BSD-compat)       │
├──────────────────────────────────────┤
│  TCP  │  UDP  │  RAW  │  UNIX DOMAIN │
├──────────────────────────────────────┤
│           IPv4 / IPv6                │
├──────────────────────────────────────┤
│  ARP  │  ICMP  │  IGMP  │  NDISC    │
├──────────────────────────────────────┤
│       Network Interface Layer        │
│  Ethernet │ Wi-Fi │ Loopback │ VPN   │
└──────────────────────────────────────┘
```

---

## Sicurezza

Architettura di sicurezza a strati:

1. **Hardware** — Secure Boot, IOMMU, TPM 2.0
2. **Kernel** — ASLR, KASLR, SMEP/SMAP, NX
3. **Sandbox** — Ogni app in namespace isolato
4. **Crypto** — AES-256, ChaCha20, Ed25519 per firme
5. **Network** — Firewall stateful, IDS integrato
6. **Updates** — Firme digitali, rollback, atomic update

---

## Desktop Environment (Wiri)

Il DE si chiama **Wiri** (thai: วิริยะ — perseveranza).

```
┌────────────────────────────────────────────────────┐
│  Compositor (Wayland) — rendering GPU-accelerato   │
│  ┌──────┐ ┌──────────┐ ┌──────┐ ┌──────┐          │
│  │ Dock │ │ Task Bar │ │Start │ │Notif │          │
│  └──────┘ └──────────┘ └──────┘ └──────┘          │
│  ┌──────────────────────────────────────────────┐  │
│  │              Workspace Manager               │  │
│  │  ┌────┐ ┌────┐ ┌────┐ ┌────┐ ┌────┐        │  │
│  │  │ W1 │ │ W2 │ │ W3 │ │ W4 │ │ W5 │        │  │
│  │  └────┘ └────┘ └────┘ └────┘ └────┘        │  │
│  └──────────────────────────────────────────────┘  │
└────────────────────────────────────────────────────┘
```

---

## Roadmap

| Milestone | Componenti | Stato |
|-----------|-----------|-------|
| M0 - Boot | Bootloader, entry point, serial output | 📝 |
| M1 - Kernel base | MM, scheduler, syscall base | 📝 |
| M2 - Drivers | PCI, ATA, NVMe, USB | 📝 |
| M3 - File System | VFS + ThaiFS + ext4 read | 📝 |
| M4 - Network | IPv4, TCP, socket API | 📝 |
| M5 - Userland | libc, loader, shell | 📝 |
| M6 - GUI | Compositor, Wayland, Wiri | 📝 |
| M7 - Apps | ThaiFiles, ThaiTerminal, ThaiStore | 📝 |
| M8 - Security | Sandbox, firewall, crypto | 📝 |
| M9 - ThaiAI | NLP, recognition, automation | 📝 |
| M10 - Release | Beta testing, ottimizzazione | 📝 |
