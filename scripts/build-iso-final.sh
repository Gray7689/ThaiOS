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
    for cmd in mksquashfs xorriso grub-mkimage mkfs.fat mmd mcopy; do
        if ! command -v "$cmd" &>/dev/null; then
            missing+=("$cmd")
        fi
    done
    
    if [ ${#missing[@]} -gt 0 ]; then
        log "Installazione dipendenze mancanti..."
        apt-get update
        apt-get install -y squashfs-tools xorriso grub-pc-bin \
            grub-efi-amd64-bin mtools dosfstools \
            isolinux syslinux-common 2>/dev/null || \
        apt-get install -y squashfs-tools xorriso grub-pc-bin \
            grub-efi-amd64-bin mtools dosfstools
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
    local vmlinuz
    vmlinuz=$(ls "$ROOTFS_DIR/boot/vmlinuz-"* 2>/dev/null | head -1)
    if [ -f "$vmlinuz" ]; then
        cp "$vmlinuz" "$ISO_DIR/boot/vmlinuz"
    fi
    local initrd
    initrd=$(ls "$ROOTFS_DIR/boot/initrd.img-"* 2>/dev/null | head -1)
    if [ -f "$initrd" ]; then
        cp "$initrd" "$ISO_DIR/boot/initrd.img"
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
  set root=(cd)
  linux /boot/vmlinuz boot=live live-media-path=/live quiet splash --
  initrd /boot/initrd.img
}

menuentry "ThaiOS 1.0 - Modalita provvisoria" {
  set root=(cd)
  linux /boot/vmlinuz boot=live live-media-path=/live nomodeset --
  initrd /boot/initrd.img
}

menuentry "Verifica memoria" {
  linux /boot/memtest86+
}
GRUBCFG
    
    # ISOLINUX config
    cat > "$ISO_DIR/isolinux/isolinux.cfg" << 'ISOCFG'
DEFAULT thaios
PROMPT 1
TIMEOUT 100
UI vesamenu.c32
MENU TITLE ThaiOS 1.0 - Songkran
MENU COLOR border 0 #FFFFFFFF #00000000 none
MENU COLOR title 0 #FF6B35 #00000000 none
MENU COLOR sel 0 #FFFFFFFF #1A274488 none

LABEL thaios
  MENU LABEL Avvia ThaiOS 1.0
  KERNEL /boot/vmlinuz
  APPEND initrd=/boot/initrd.img boot=live live-media-path=/live quiet splash

LABEL thaios-safe
  MENU LABEL Modalita provvisoria
  KERNEL /boot/vmlinuz
  APPEND initrd=/boot/initrd.img boot=live live-media-path=/live nomodeset

LABEL hdd
  MENU LABEL Avvia dal disco locale
  LOCALBOOT 0x80
ISOCFG
    
    # Copy boot splash
    if [ -f "$THAIOS_ROOT/branding/boot/splash.png" ]; then
        cp "$THAIOS_ROOT/branding/boot/splash.png" "$ISO_DIR/boot/thaios-splash.png"
    fi
    
    # Copy isolinux binaries
    if [ -f /usr/lib/ISOLINUX/isolinux.bin ]; then
        cp /usr/lib/ISOLINUX/isolinux.bin "$ISO_DIR/isolinux/"
    fi
    if [ -f /usr/lib/syslinux/modules/bios/vesamenu.c32 ]; then
        cp /usr/lib/syslinux/modules/bios/vesamenu.c32 "$ISO_DIR/isolinux/"
    fi
    if [ -f /usr/lib/syslinux/modules/bios/libcom32.c32 ]; then
        cp /usr/lib/syslinux/modules/bios/libcom32.c32 "$ISO_DIR/isolinux/"
    fi
    if [ -f /usr/lib/syslinux/modules/bios/libutil.c32 ]; then
        cp /usr/lib/syslinux/modules/bios/libutil.c32 "$ISO_DIR/isolinux/"
    fi
    
    # Create UEFI boot image
    log "Creazione immagine EFI..."
    grub-mkimage -o "$ISO_DIR/boot/grub/BOOTx64.EFI" \
        -p /boot/grub -O x86_64-efi \
        part_gpt part_msdos iso9660 squash4 loopback ext2 \
        configfile normal boot efi_gop efi_uga \
        search search_fs_file ls cat echo test video font gfxterm gfxmenu \
        gfxterm_background png jpeg all_video || true
    if [ -f "$ISO_DIR/boot/grub/BOOTx64.EFI" ]; then
        dd if=/dev/zero bs=1M count=4 of="$ISO_DIR/boot/grub/efi.img" 2>/dev/null
        mkfs.fat "$ISO_DIR/boot/grub/efi.img" >/dev/null 2>&1
        mmd -i "$ISO_DIR/boot/grub/efi.img" EFI EFI/BOOT >/dev/null 2>&1
        mcopy -i "$ISO_DIR/boot/grub/efi.img" "$ISO_DIR/boot/grub/BOOTx64.EFI" ::EFI/BOOT/BOOTx64.EFI >/dev/null 2>&1
        log "Immagine EFI creata con successo"
    else
        log "AVVISO: GRUB EFI non generato, ISO solo BIOS"
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
    log "Fase 3: Generazione ISO finale..."
    
    # Cleanup old ISO
    rm -f "$OUTPUT_ISO"
    
    # Build hybrid ISO with xorriso (BIOS + UEFI)
    xorriso -as mkisofs \
        -iso-level 3 \
        -full-iso9660-filenames \
        -volid "ThaiOS-1-0" \
        -appid "ThaiOS Live" \
        -publisher "ThaiOS" \
        -eltorito-boot isolinux/isolinux.bin \
        -eltorito-catalog isolinux/boot.cat \
        -no-emul-boot -boot-load-size 4 -boot-info-table \
        -eltorito-alt-boot -e boot/grub/efi.img -no-emul-boot \
        -isohybrid-gpt-basdat \
        -isohybrid-apm-hfsplus \
        -output "$OUTPUT_ISO" \
        "$ISO_DIR" 2>&1 | grep -v "ISO:"



    
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
