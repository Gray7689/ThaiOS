# Architettura di ThaiOS

## Panoramica

ThaiOS e' un sistema operativo moderno costruito su un kernel Linux con un desktop environment proprietario chiamato ThaiDesktop. L'architettura e' progettata per offrire isolamento, performance e un'esperienza utente coerente.

## Struttura a livelli

```
+--------------------------------------------------+
|              Applicazioni ThaiOS                  |
|  ThaiBrowser | ThaiFiles | ThaiStore | ...        |
+--------------------------------------------------+
|              ThaiDesktop Environment              |
|  ThaiPanel | ThaiDock | ThaiAppMenu | ...         |
+--------------------------------------------------+
|            ThaiDesktop Compositor                 |
|         (Wayland compositor nativo)               |
+--------------------------------------------------+
|               System Services                     |
|  ThaiPkg | ThaiUpdater | ThaiInstaller | ...      |
+--------------------------------------------------+
|               Kernel ThaiOS                       |
|  (Linux kernel 6.6.x con patch ThaiOS)            |
+--------------------------------------------------+
|                    Hardware                       |
+--------------------------------------------------+
```

## Componenti principali

### 1. Kernel ThaiOS
- Basato su Linux kernel 6.6.x LTS
- Patch ThaiOS per boot ottimizzato e branding
- Moduli: ext4, Btrfs, NTFS, ZFS
- Supporto: x86_64, ARM64

### 2. ThaiDesktop Compositor
- Wayland compositor nativo basato su wlroots
- Gestione scene graph per rendering ottimizzato
- Supporto layer-shell per panel e dock
- Animazioni GPU-accelerate
- Temi nativi con schema colori personalizzato

### 3. ThaiDesktop Components
- **ThaiShell**: Session manager che orchestra tutti i componenti
- **ThaiPanel**: Barra superiore con menu, orologio e tray
- **ThaiDock**: Dock moderno con app preferite e app in esecuzione
- **ThaiAppMenu**: Lanciatore applicazioni con ricerca e categorie
- **ThaiNotification**: Centro notifiche a scomparsa
- **ThaiControlCenter**: Pannello controllo rapido
- **ThaiWidgets**: Engine per widget desktop

### 4. Applicazioni ThaiOS
Tutte le applicazioni seguono il Thai Design System con:
- Stessa palette colori
- Stessi componenti UI
- Stessa tipografia (Sarabun)
- Animazioni consistenti
- Supporto tema chiaro/scuro

### 5. ThaiPkg
- Package manager transazionale
- Database pacchetti in JSON
- Risoluzione dipendenze automatica
- Repository firmati GPG
- Supporto pacchetti .tai (ThaiOS Installer)

### 6. ThaiUpdater
- Aggiornamenti OTA incrementali
- Verifica firme digitali
- Rollback automatico su fallimento
- Notifiche desktop
- Pianificazione aggiornamenti

## Sicurezza

- Sandboxing delle applicazioni con namespaces
- Permessi granulari stile mobile
- Crittografia disco completa (LUKS)
- Firewall preconfigurato
- Aggiornamenti di sicurezza automatici
- Firmaware verification all'avvio

## Thai Design System

### Colori
- **Primario**: #FF6B35 (Arancione Thai)
- **Secondario**: #1D3557 (Blu Thai)
- **Accento**: #E63946 (Rosso Thai)
- **Superficie**: #1A2744 / #F5F7FA
- **Testo**: #E8EDF5 / #1D3557

### Tipografia
- **Font**: Sarabun (ThaiOS Custom)
- **Pesi**: ExtraLight, Regular, Medium, Bold
- **Dimensioni**: 10-36px

### Spaziatura
- **Unit**: 4px
- **Card padding**: 16px
- **Border radius**: 12px
- **Gap standard**: 8px
