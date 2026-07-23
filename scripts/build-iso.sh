#!/bin/bash
#
# ThaiOS ISO Generator
#

set -e

BUILD_DIR="${1:-build}"
THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ISO_DIR="$BUILD_DIR/iso"
DESTDIR="$BUILD_DIR/rootfs"

log() { echo -e "\033[1;32m[ThaiOS]\033[0m $*"; }
error() { echo -e "\033[1;31m[ERROR]\033[0m $*"; exit 1; }

log "Generazione ISO ThaiOS..."

mkdir -p "$ISO_DIR/boot/grub"
mkdir -p "$ISO_DIR/ThaiOS"

if [ -d "$DESTDIR" ]; then
    cp -a "$DESTDIR"/* "$ISO_DIR/ThaiOS/" 2>/dev/null || true
fi

# Boot splash
if [ -f "$THAIOS_ROOT/branding/boot/splash.png" ]; then
    cp "$THAIOS_ROOT/branding/boot/splash.png" "$ISO_DIR/boot/"
fi

# GRUB configuration
cat > "$ISO_DIR/boot/grub/grub.cfg" << 'GRUB'
set default=0
set timeout=5
set gfxmode=auto
set gfxpayload=keep

insmod all_video
insmod gfxterm
insmod png
insmod part_gpt
insmod ext2

if [ -f /boot/splash.png ]; then
    background_image /boot/splash.png
fi

menuentry "ThaiOS 1.0" {
    linux /ThaiOS/boot/vmlinuz root=/dev/sda1 ro quiet splash
    initrd /ThaiOS/boot/initrd.img
}

menuentry "ThaiOS - Modalit\u00e0 provvisoria" {
    linux /ThaiOS/boot/vmlinuz root=/dev/sda1 ro nomodeset
    initrd /ThaiOS/boot/initrd.img
}

menuentry "ThaiOS - Ripristino" {
    linux /ThaiOS/boot/vmlinuz root=/dev/sda1 ro single
    initrd /ThaiOS/boot/initrd.img
}
GRUB

# Create ISO
if command -v xorriso &> /dev/null; then
    log "Generazione ISO con xorriso..."
    xorriso -as mkisofs \
        -iso-level 3 \
        -full-iso9660-filenames \
        -volid "ThaiOS-1-0" \
        -appid "ThaiOS Installer" \
        -publisher "ThaiOS" \
        -eltorito-boot boot/grub/isolinux.bin \
        -eltorito-catalog boot/grub/boot.cat \
        -no-emul-boot \
        -boot-load-size 4 \
        -boot-info-table \
        -isohybrid-gpt-basdat \
        -output "$BUILD_DIR/ThaiOS-1.0.iso" \
        "$ISO_DIR"
    log "ISO pronta: $BUILD_DIR/ThaiOS-1.0.iso"
elif command -v mkisofs &> /dev/null; then
    mkisofs -o "$BUILD_DIR/ThaiOS-1.0.iso" -b boot/grub/isolinux.bin -c boot/grub/boot.cat \
        -no-emul-boot -boot-load-size 4 -boot-info-table -J -R -V "ThaiOS-1-0" "$ISO_DIR"
    log "ISO pronta: $BUILD_DIR/ThaiOS-1.0.iso"
else
    error "xorriso o mkisofs non trovato. Installare genisoimage o xorriso."
fi
