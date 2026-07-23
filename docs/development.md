# Sviluppo di ThaiOS

## Prerequisiti

### Compilazione
- GCC/Clang
- GNU Make
- Python 3.8+
- pkg-config
- wayland-protocols
- wlroots 0.17+
- GTK3 + PyGObject
- WebKitGTK (per ThaiBrowser)
- VTE (per ThaiTerminal)
- GtkSourceView (per ThaiEditor)
- xorriso (per ISO)

### Sviluppo
- Git
- Valgrind
- gdb
- meson (opzionale)

## Setup iniziale

```bash
# Clona il repository
git clone https://github.com/ThaiOS/thaios.git
cd thaios

# Configura
./configure --prefix=/usr

# Compila
make -j$(nproc)
```

## Struttura del progetto

```
thaios/
├── apps/              # Applicazioni ThaiOS
│   ├── ThaiBrowser/
│   ├── ThaiFiles/
│   ├── ThaiStore/
│   ├── ThaiTerminal/
│   ├── ThaiSettings/
│   ├── ThaiMedia/
│   ├── ThaiEditor/
│   └── ThaiBackup/
├── base/              # File di sistema
├── branding/          # Asset grafici
├── compositor/        # ThaiDesktop compositor
├── desktop/           # Componenti GUI
│   ├── panel/
│   ├── dock/
│   ├── app-menu/
│   ├── notification-center/
│   ├── control-center/
│   └── widgets/
├── docs/              # Documentazione
├── kernel/            # Config kernel
├── scripts/           # Script di build
├── thai-installer/    # Installer
├── thai-pkg/          # Package manager
└── thai-updater/      # Update system
```

## Convenzioni di Codice

### Python
- Seguire PEP 8
- Documentare con docstring
- Usare type hints dove possibile
- Nomi variabili in snake_case
- Nomi classi in PascalCase

### C (Compositor)
- Seguire lo stile del kernel Linux
- Nomi funzione: snake_case con prefisso `thai_`
- Struct: typedef con prefisso `struct thai_`
- Header: header guard con `THAI_` prefisso

### GTK/CSS
- ID widget: kebab-case con prefisso thai
- Classi CSS: kebab-case
- Colori in esadecimale

## Test

```bash
# Esegui test componenti
make test

# Test ThaiPkg
thai-pkg --test

# Test isolato compositor
./build/compositor/thai-desktop --test
```

## Debug

```bash
# Compositor in debug mode
./build/compositor/thai-desktop -d

# Logging
WLR_BACKENDS=headless ./build/compositor/thai-desktop

# Valgrind
valgrind ./build/compositor/thai-desktop
```

## Contribuire

1. Fork del repository
2. Crea un branch: `git checkout -b feature/nome-feature`
3. Commit con messaggi descrittivi
4. Pull request con descrizione dettagliata

## Release

```bash
# Build completa
./scripts/build.sh all

# Genera ISO
./scripts/build-iso.sh

# Firma release
gpg --sign ThaiOS-1.0.iso
```
