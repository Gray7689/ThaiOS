# Building ThaiOS

## Prerequisiti

### Toolchain cross-compilazione (necessaria)
```bash
# Installare dipendenze (Debian/Ubuntu)
sudo apt install build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev \
                 texinfo nasm xorriso qemu-system-x86 mtools

# Configurare toolchain
make setup
```

### In alternativa: usare GCC cross precompilato
```bash
export PATH=/opt/x86_64-elf-gcc/bin:$PATH
```

## Compilazione

```bash
# Build completo
make all

# Solo kernel (debug rapido)
make kernel

# Genera ISO avviabile
make iso

# Avvia in QEMU
make run-qemu
```

## Struttura output

```
build/
├── kernel.elf       # Kernel eseguibile
├── initramfs.img    # Init ramfs
├── sysroot/         # Root filesystem di ThaiOS
│   ├── bin/         # Binari utente
│   ├── lib/         # Librerie
│   ├── usr/         # Programmi utente
│   └── etc/         # Configurazioni
├── ThaiOS.iso       # ISO avviabile
└── modules/         # Kernel modules (.ko)
```

## Debug

```bash
# QEMU + GDB
qemu-system-x86_64 -cdrom build/ThaiOS.iso -s -S &
gdb build/kernel.elf -ex "target remote :1234" -ex "break kmain" -ex "continue"

# Log seriale
qemu-system-x86_64 -cdrom build/ThaiOS.iso -serial stdio
```

## Test

```bash
make test            # Esegue tutti i test
make test-kernel     # Test kernel unit
make test-fs         # Test filesystem
make test-net        # Test networking
```
