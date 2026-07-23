# Guida Utente ThaiOS

## Benvenuto in ThaiOS

ThaiOS e' un sistema operativo moderno progettato per essere intuitivo, veloce e sicuro. Questa guida ti aiuterà a familiarizzare con le funzionalità principali.

## Primo Avvio

### Schermata di Login
All'avvio, ThaiOS presenta una schermata di login elegante con:
- Campo nome utente
- Campo password
- Selettore sessione (ThaiOS Desktop)
- Opzioni di accessibilita'

### ThaiDesktop
Dopo il login, verrai accolto da ThaiDesktop:
- **ThaiPanel**: Barra superiore con menu applicazioni, orologio e stato sistema
- **ThaiDock**: Dock inferiore con applicazioni preferite
- **Scrivania**: Area di lavoro con widget personalizzabili

## Interfaccia Utente

### ThaiPanel (Barra Superiore)
- **Menu ThaiOS** (sinistra): Apre ThaiAppMenu
- **Workspace**: Indicatore spazi di lavoro
- **Orologio**: Data e ora correnti
- **Tray di sistema**: Connessioni, volume, batteria
- **Centro notifiche**: Icona campana per notifiche
- **Controllo rapido**: Wi-Fi, Bluetooth, luminosita'

### ThaiDock (Dock)
- Applicazioni preferite sempre visibili
- Indicatore applicazioni in esecuzione
- Click per avviare o portare in primo piano
- Click destro per opzioni aggiuntive

### ThaiAppMenu (Menu Applicazioni)
- Cerca applicazioni con barra di ricerca
- Categorie organizzate
- Scorciatoie da tastiera

## Applicazioni Native

### ThaiBrowser
- Navigazione web con WebKit
- Schede multiple
- Segnalibri
- Cronologia
- Modalita' privata

### ThaiFiles
- Navigazione file system
- Vista griglia e lista
- Sidebar con raccolte
- Ricerca file
- Anteprima file

### ThaiStore
- Esplora applicazioni
- Installa con un click
- Aggiornamenti automatici
- Categorie e ricerca

### ThaiTerminal
- Emulatore terminale VTE
- Multipiattaforma
- Profili colore personalizzabili
- Scorciatoie da tastiera

### ThaiSettings
- Impostazioni sistema
- Personalizzazione temi
- Gestione account
- Rete e dispositivi
- Aggiornamenti

## Personalizzazione

### Temi
1. Apri ThaiSettings
2. Vai a "Personalizzazione" > "Temi"
3. Scegli tra ThaiOS Light, ThaiOS Dark o temi aggiuntivi
4. Personalizza colore accento

### Sfondo
1. Click destro sulla scrivania
2. Seleziona "Cambia sfondo"
3. Scegli tra sfondi predefiniti o immagine personalizzata

### Dock
1. ThaiSettings > Personalizzazione > Dock
2. Modifica dimensione, posizione e app preferite

## Gestione Pacchetti

### ThaiPkg (Terminale)
```bash
# Cerca un pacchetto
thai-pkg search <nome>

# Installa un pacchetto
sudo thai-pkg install <nome>

# Rimuovi un pacchetto
sudo thai-pkg remove <nome>

# Aggiorna repository
sudo thai-pkg update

# Aggiorna tutto il sistema
sudo thai-pkg upgrade
```

### ThaiStore (GUI)
- Apri ThaiStore dall dock o dal menu
- Sfoglia le categorie
- Clicca "Installa" per installare
- ThaiStore gestisce automaticamente le dipendenze

## Aggiornamenti

ThaiUpdater mantiene il sistema aggiornato:
- Notifiche automatiche per aggiornamenti disponibili
- Aggiornamenti di sicurezza prioritari
- Installazione con un click
- Cronologia aggiornamenti

## Backup

ThaiBackup protegge i tuoi dati:
- Backup automatici pianificati
- Backup manuali
- Ripristino selettivo
- Destinazioni multiple (disco locale, NAS, cloud)

## Scorciatoie da Tastiera

| Combinazione | Azione |
|-------------|--------|
| Win | Apri ThaiAppMenu |
| Win+D | Mostra scrivania |
| Win+L | Blocca schermo |
| Win+Tab | Cambia applicazione |
| Win+Freccia | Snap finestra |
| Ctrl+Alt+T | Apri ThaiTerminal |
| Print | Screenshot |
| Win+Print | Screenshot area |

## Supporto

- **Documentazione**: docs.thaios.dev
- **Forum**: community.thaios.dev
- **Bug**: bugs.thaios.dev
- **Email**: support@thaios.dev

## Risoluzione Problemi

### ThaiDesktop non si avvia
1. Premi Ctrl+Alt+F2 per terminale testuale
2. Login con le tue credenziali
3. Esegui: `sudo thai-pkg reinstall ThaiDesktop`
4. Riavvia: `sudo reboot`

### Reset password
1. Avvia ThaiOS in modalita' provvisoria dal boot
2. Seleziona "ThaiOS - Ripristino"
3. Esegui: `passwd <nome_utente>`
4. Riavvia

### Connessione di rete
1. Apri ThaiSettings > Rete
2. Verifica Wi-Fi sia attivo
3. Seleziona la rete desiderata
4. Inserisci la password
