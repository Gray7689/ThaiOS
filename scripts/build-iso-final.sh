#!/bin/bash
#
# ThaiOS Final ISO Builder
# Creates a bootable live ISO with ThaiOS
#

set -e

THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$THAIOS_ROOT/build"
ROOTFS_DIR="$BUILD_DIR/rootfs"
ISO_DIR="$BUILD_DIR/iso"
ROOTFS_SQUASH="$BUILD_DIR/filesystem.squashfs"
OUTPUT_ISO="$BUILD_DIR/ThaiOS-1.0.iso"
THAIOS_VERSION="1.0"
THAIOS_CODENAME="Songkran"

log() { echo -e "\033[1;32m[ThaiOS]\033[0m $*"; }
error() { echo -e "\033[1;31m[ERROR]\033[0m $*"; exit 1; }

check_deps() {
    local missing=()
    for cmd in mksquashfs grub-mkrescue xorriso; do
        if ! command -v "$cmd" &>/dev/null; then
            missing+=("$cmd")
        fi
    done
    
    if [ ${#missing[@]} -gt 0 ]; then
        log "Installazione dipendenze mancanti: ${missing[*]}..."
        apt-get update -qq 2>/dev/null || true
        apt-get install -y squashfs-tools xorriso grub-pc-bin \
            grub-efi-amd64-bin grub-common 2>/dev/null || true
        local still_missing=()
        for cmd in "${missing[@]}"; do
            command -v "$cmd" &>/dev/null || still_missing+=("$cmd")
        done
        if [ ${#still_missing[@]} -gt 0 ]; then
            log "AVVISO: dipendenze non disponibili: ${still_missing[*]}"
        fi
    fi
}

build_squashfs() {
    log "Fase 1: Creazione filesystem squashfs..."
    
    if [ ! -d "$ROOTFS_DIR" ]; then
        error "Rootfs non trovato in $ROOTFS_DIR. Esegui prima build-rootfs.sh"
    fi
    
    rm -f "$ROOTFS_SQUASH"
    
    # Create squashfs with compression
    mksquashfs "$ROOTFS_DIR" "$ROOTFS_SQUASH" \
        -comp zstd \
        -Xcompression-level 3 \
        -b 1M \
        -no-recovery \
        -noappend \
        -no-progress
    
    local size=$(du -h "$ROOTFS_SQUASH" | cut -f1)
    log "Squashfs creato: $size"
}

prepare_iso_dir() {
    log "Fase 2: Preparazione directory ISO..."
    
    rm -rf "$ISO_DIR"
    mkdir -p "$ISO_DIR"/{boot/grub,live,isolinux}
    
    # Copy squashfs
    cp "$ROOTFS_SQUASH" "$ISO_DIR/live/filesystem.squashfs"
    
    # Copy kernel and initrd from rootfs
    local vmlinuz=""
    for f in "$ROOTFS_DIR/boot/vmlinuz-"*; do
        [ -f "$f" ] && { vmlinuz="$f"; break; }
    done
    if [ -n "$vmlinuz" ]; then
        cp "$vmlinuz" "$ISO_DIR/boot/vmlinuz"
    else
        error "Kernel non trovato in $ROOTFS_DIR/boot/"
    fi
    local initrd=""
    for f in "$ROOTFS_DIR/boot/initrd.img-"*; do
        [ -f "$f" ] && { initrd="$f"; break; }
    done
    if [ -n "$initrd" ]; then
        cp "$initrd" "$ISO_DIR/boot/initrd.img"
    else
        error "Initrd non trovato in $ROOTFS_DIR/boot/"
    fi
    
    # Create GRUB rescue config
    cat > "$ISO_DIR/boot/grub/grub.cfg" << 'GRUBCFG'
insmod all_video
insmod gfxterm
insmod png
insmod loopback
insmod squash4
insmod iso9660
insmod ext2
insmod part_gpt
insmod part_msdos

set default=0
set timeout=5
set gfxmode=1920x1080
set gfxpayload=keep
loadfont unicode

if [ -e /boot/thaios-splash.png ]; then
  background_image /boot/thaios-splash.png
fi

menuentry "Avvia ThaiOS 1.0" --class thaios {
  linux /boot/vmlinuz boot=live live-media-path=/live quiet splash --
  initrd /boot/initrd.img
}

menuentry "ThaiOS 1.0 - Modalita provvisoria" {
  linux /boot/vmlinuz boot=live live-media-path=/live nomodeset --
  initrd /boot/initrd.img
}

menuentry "Verifica memoria" {
  linux /boot/memtest86+
}
GRUBCFG
    
    # Copy boot splash (convert from SVG if needed)
    if [ -f "$THAIOS_ROOT/branding/boot/splash.png" ]; then
        cp "$THAIOS_ROOT/branding/boot/splash.png" "$ISO_DIR/boot/thaios-splash.png"
    elif [ -f "$THAIOS_ROOT/branding/boot/splash.svg" ]; then
        if command -v rsvg-convert &>/dev/null; then
            rsvg-convert "$THAIOS_ROOT/branding/boot/splash.svg" -o "$ISO_DIR/boot/thaios-splash.png" 2>/dev/null || true
        elif command -v convert &>/dev/null; then
            convert "$THAIOS_ROOT/branding/boot/splash.svg" "$ISO_DIR/boot/thaios-splash.png" 2>/dev/null || true
        fi
    fi
    
    # Create a README on the ISO
    cat > "$ISO_DIR/README.TXT" << 'README'
ThaiOS 1.0 (Songkran)
=====================
Sistema operativo moderno e sicuro.

Per installare ThaiOS sul tuo computer:
  - Avvia dal vivo
  - Apri ThaiInstaller (thai-installer)
  - Segui la procedura guidata

https://thaios.dev
README
    
    log "Directory ISO preparata"
}

generate_iso() {
    log "Fase 3: Generazione ISO finale con grub-mkrescue..."
    
    rm -f "$OUTPUT_ISO"
    
    # grub-mkrescue creates a proper hybrid BIOS+UEFI ISO automatically
    # It handles: GRUB BIOS core.img, GRUB UEFI efi.img, hybrid MBR/GPT, El Torito
    grub-mkrescue -o "$OUTPUT_ISO" \
        "$ISO_DIR" \
        -- -volid "ThaiOS-1-0" -appid "ThaiOS Live" -publisher "ThaiOS"
    
    if [ -f "$OUTPUT_ISO" ]; then
        local size=$(du -h "$OUTPUT_ISO" | cut -f1)
        log "ISO ThaiOS generata: $OUTPUT_ISO ($size)"
        log "SHA256: $(sha256sum "$OUTPUT_ISO" | cut -d' ' -f1)"
    else
        error "Fallita generazione ISO"
    fi
}

show_help() {
    cat << HELP
ThaiOS ISO Builder
==================
Uso: $0 [comando]

Comandi:
  all          Build completo (default)
  squashfs     Crea solo il filesystem squashfs
  iso          Prepara directory e genera ISO
  help         Mostra questo aiuto

Prerequisiti:
  - build-rootfs.sh deve essere stato eseguito prima
  - pacchetti: squashfs-tools xorriso grub-pc-bin
HELP
}

# Main
case "${1:-all}" in
    all)
        check_deps
        build_squashfs
        prepare_iso_dir
        generate_iso
        log "Build completa! ISO: $OUTPUT_ISO"
        ;;
    squashfs) build_squashfs ;;
    iso) 
        check_deps
        prepare_iso_dir
        generate_iso
        ;;
    help|--help|-h) show_help ;;
    *)
        error "Comando sconosciuto: $1"
        show_help
        exit 1
        ;;
esac
