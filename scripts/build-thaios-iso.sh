#!/bin/bash
#
# ThaiOS Build Full ISO
# Unico script per buildare ThaiOS completo dalla sorgente
#

set -e

THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$THAIOS_ROOT/build"
THAIOS_VERSION="1.0"
THAIOS_CODENAME="Songkran"

log() { echo -e "\033[1;32m[ThaiOS]\033[0m $*"; }
error() { echo -e "\033[1;31m[ERROR]\033[0m $*"; exit 1; }

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

check_system() {
    log "Verifica ambiente di build..."
    
    if [ "$(id -u)" -ne 0 ]; then
        error "Questo script deve essere eseguito come root (sudo)"
    fi
    
    # Check architecture
    if [ "$(uname -m)" != "x86_64" ]; then
        error "Supportata solo architettura x86_64"
    fi
    
    # Check internet
    if ! ping -c 1 8.8.8.8 &>/dev/null; then
        error "Nessuna connessione internet. Richiesto per debootstrap."
    fi
    
    log "Sistema: $(uname -a | head -1)"
    log "Build: ThaiOS $THAIOS_VERSION ($THAIOS_CODENAME)"
}

build_everything() {
    log "================================================"
    log " Build ThaiOS $THAIOS_VERSION ISO"
    log "================================================"
    echo ""
    
    # Step 1: Build rootfs
    log "${YELLOW}[1/4]${NC} Build del rootfs ThaiOS..."
    bash "$THAIOS_ROOT/scripts/build-rootfs.sh" all
    echo ""
    
    # Step 2: Install ThaiOS source apps
    log "${YELLOW}[2/4]${NC} Installazione applicazioni nel rootfs..."
    bash "$THAIOS_ROOT/scripts/build-rootfs.sh" apps
    bash "$THAIOS_ROOT/scripts/build-rootfs.sh" desktop
    bash "$THAIOS_ROOT/scripts/build-rootfs.sh" branding
    bash "$THAIOS_ROOT/scripts/build-rootfs.sh" finalize
    echo ""
    
    # Step 3: Build ISO
    log "${YELLOW}[3/4]${NC} Generazione ISO avviabile..."
    bash "$THAIOS_ROOT/scripts/build-iso-final.sh" all
    echo ""
    
    # Step 4: Verify
    log "${YELLOW}[4/4]${NC} Verifica ISO..."
    
    if [ -f "$BUILD_DIR/ThaiOS-1.0.iso" ]; then
        local size=$(du -h "$BUILD_DIR/ThaiOS-1.0.iso" | cut -f1)
        local sha=$(sha256sum "$BUILD_DIR/ThaiOS-1.0.iso" | cut -d' ' -f1)
        
        echo ""
        log "================================================"
        log "${GREEN} BUILD COMPLETATO CON SUCCESSO${NC}"
        log "================================================"
        echo ""
        log "  ISO: ${GREEN}$BUILD_DIR/ThaiOS-1.0.iso${NC}"
        log "  Dimensione: $size"
        log "  SHA256: $sha"
        echo ""
        log "Per installare:"
        log "  1. Copia l'ISO su una chiavetta USB:"
        log "     ${YELLOW}dd if=$BUILD_DIR/ThaiOS-1.0.iso of=/dev/sdX bs=4M status=progress${NC}"
        log "  2. Avvia dal dispositivo USB"
        log "  3. Segui ThaiInstaller"
        echo ""
    else
        error "ISO non trovata!"
    fi
}

clean_all() {
    log "Pulizia completa..."
    rm -rf "$BUILD_DIR"
    log "Pulizia completata"
}

show_help() {
    cat << HELP
ThaiOS Build System — ISO Creator
==================================
Uso: sudo $0 [comando]

Comandi:
  all         Build completo ISO (default)
  clean       Pulisce tutti i file di build
  rootfs      Solo build del rootfs
  iso         Solo generazione ISO (dopo rootfs)
  help        Mostra questo aiuto

Requisiti:
  - Debian/Ubuntu x86_64
  - Connessione internet
  - Eseguire come root (sudo)
HELP
}

# Main
case "${1:-all}" in
    all)
        check_system
        build_everything
        ;;
    rootfs)
        check_system
        bash "$THAIOS_ROOT/scripts/build-rootfs.sh" all
        bash "$THAIOS_ROOT/scripts/build-rootfs.sh" apps
        bash "$THAIOS_ROOT/scripts/build-rootfs.sh" desktop
        bash "$THAIOS_ROOT/scripts/build-rootfs.sh" branding
        bash "$THAIOS_ROOT/scripts/build-rootfs.sh" finalize
        log "Rootfs pronto in $BUILD_DIR/rootfs"
        ;;
    iso)
        check_system
        bash "$THAIOS_ROOT/scripts/build-iso-final.sh" all
        ;;
    clean)
        clean_all
        ;;
    help|--help|-h)
        show_help
        ;;
    *)
        error "Comando sconosciuto: $1"
        show_help
        exit 1
        ;;
esac
