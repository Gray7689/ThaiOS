# ThaiOS

**ThaiOS** è un sistema operativo moderno, veloce e sicuro, costruito su un kernel robusto e progettato per offrire un'esperienza utente fluida e professionale.

## Panoramica

ThaiOS nasce con l'obiettivo di offrire un sistema operativo completo, elegantemente progettato e facile da usare, pur mantenendo solide fondamenta tecniche. L'interfaccia utente originale e le applicazioni native con marchio ThaiOS garantiscono un'esperienza coesa e produttiva.

## Caratteristiche principali

- **Interfaccia originale** — Desktop environment proprietario ThaiDesktop con dock moderno, barra superiore, centro notifiche e controllo
- **Applicazioni native** — ThaiBrowser, ThaiFiles, ThaiStore, ThaiTerminal, ThaiSettings, ThaiMedia, ThaiEditor, ThaiBackup
- **ThaiPkg** — Sistema di gestione pacchetti veloce e affidabile
- **Aggiornamenti OTA** — ThaiUpdater mantiene il sistema sempre aggiornato
- **Sicurezza integrata** — Sandboxing, permessi granulari, crittografia nativa
- **Performance ottimizzate** — Boot rapido, basso consumo di risorse, animazioni fluide

## Requisiti di sistema

| Componente | Minimo | Consigliato |
|------------|--------|-------------|
| Processore | x86_64, 2 core | x86_64, 4+ core |
| RAM | 2 GB | 4+ GB |
| Archiviazione | 16 GB | 64+ GB (SSD) |
| Schermo | 1024x768 | 1920x1080+ |

## Installazione

1. Scarica l'ISO di ThaiOS dal sito ufficiale
2. Crea una chiavetta USB avviabile con ThaiOS Installer o `dd`
3. Avvia dal dispositivo USB
4. Segui la procedura guidata di ThaiOS Installer

## Build da sorgente

```bash
git clone https://github.com/ThaiOS/thaios.git
cd thaios
./configure
make
sudo make install
```

## Licenza

ThaiOS è distribuito sotto licenza GNU General Public License v2.0.
Vedere il file [LICENSE](LICENSE) per i dettagli.
