#!/bin/bash
#
# ThaiOS Build Script
# Builds the entire ThaiOS system from source
#

set -e

THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${THAIOS_ROOT}/build"
DESTDIR="${BUILD_DIR}/rootfs"
JOBS=$(nproc 2>/dev/null || echo 4)

export THAIOS_ROOT BUILD_DIR DESTDIR

log() {
    echo -e "\033[1;32m[ThaiOS]\033[0m $*"
}

error() {
    echo -e "\033[1;31m[ERROR]\033[0m $*"
    exit 1
}

clean() {
    log "Pulizia directory di build..."
    rm -rf "$BUILD_DIR"
    log "Pulizia completata"
}

prepare() {
    log "Preparazione ambiente di build..."
    mkdir -p "$BUILD_DIR" "$DESTDIR"
    
    for dir in boot etc usr/bin usr/lib usr/share/ThaiOS; do
        mkdir -p "$DESTDIR/$dir"
    done
    
    log "Ambiente preparato in $BUILD_DIR"
}

build_kernel_config() {
    log "Configurazione kernel..."
    mkdir -p "$BUILD_DIR/kernel"
    if [ -f "$THAIOS_ROOT/kernel/config" ]; then
        cp "$THAIOS_ROOT/kernel/config" "$BUILD_DIR/kernel/.config"
        log "Configurazione kernel copiata"
    else
        log "ATTENZIONE: file config kernel non trovato, generazione config predefinita"
        log "Usa 'make kernel-config' per personalizzare"
    fi
}

build_base() {
    log "Installazione file di base ThaiOS..."
    
    # os-release e file identità
    cp "$THAIOS_ROOT/base/etc/os-release" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/issue" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/lsb-release" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/motd" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/profile" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/bash.bashrc" "$DESTDIR/etc/"
    cp "$THAIOS_ROOT/base/etc/zshrc" "$DESTDIR/etc/"
    
    mkdir -p "$DESTDIR/etc/skel"
    cp "$THAIOS_ROOT/base/etc/skel/.bashrc" "$DESTDIR/etc/skel/"
    cp "$THAIOS_ROOT/base/etc/skel/.zshrc" "$DESTDIR/etc/skel/"
    
    # Session config
    mkdir -p "$DESTDIR/usr/share/ThaiOS"
    cp "$THAIOS_ROOT/base/usr/share/ThaiOS/session.conf" "$DESTDIR/usr/share/ThaiOS/"
    
    log "File base installati"
}

build_branding() {
    log "Installazione branding ThaiOS..."
    
    BRANDING_DEST="$DESTDIR/usr/share/ThaiOS"
    mkdir -p "$BRANDING_DEST/branding/logo"
    mkdir -p "$BRANDING_DEST/branding/themes"
    mkdir -p "$BRANDING_DEST/wallpapers"
    mkdir -p "$BRANDING_DEST/icons"
    mkdir -p "$BRANDING_DEST/fonts"
    mkdir -p "$BRANDING_DEST/sounds"
    mkdir -p "$DESTDIR/usr/share/icons"
    mkdir -p "$DESTDIR/usr/share/themes"
    mkdir -p "$DESTDIR/usr/share/fonts"
    mkdir -p "$DESTDIR/usr/share/sounds"
    
    # Logo
    cp "$THAIOS_ROOT/branding/logo/"*.svg "$BRANDING_DEST/branding/logo/"
    
    # Wallpapers
    cp "$THAIOS_ROOT/branding/wallpapers/"* "$BRANDING_DEST/wallpapers/"
    
    # Themes
    cp -r "$THAIOS_ROOT/branding/themes/"* "$DESTDIR/usr/share/themes/"
    cp -r "$THAIOS_ROOT/branding/themes/"* "$BRANDING_DEST/branding/themes/"
    
    # Icons
    cp -r "$THAIOS_ROOT/branding/icons/"* "$DESTDIR/usr/share/icons/"
    
    # Font config
    cp "$THAIOS_ROOT/branding/fonts/"*.json "$BRANDING_DEST/fonts/"
    
    # Sounds config
    cp "$THAIOS_ROOT/branding/sounds/"*.json "$BRANDING_DEST/sounds/"
    
    # Boot
    mkdir -p "$BRANDING_DEST/branding/boot"
    cp "$THAIOS_ROOT/branding/boot/"* "$BRANDING_DEST/branding/boot/"
    
    log "Branding installato"
}

build_apps() {
    log "Compilazione applicazioni ThaiOS..."
    
    APPS=("ThaiBrowser" "ThaiFiles" "ThaiStore" "ThaiTerminal" 
          "ThaiSettings" "ThaiMedia" "ThaiEditor" "ThaiBackup")
    
    for app in "${APPS[@]}"; do
        log "  Build $app..."
        (cd "$THAIOS_ROOT/apps/$app" && make BUILD_DIR="$BUILD_DIR/$app")
        mkdir -p "$DESTDIR/usr/bin"
        if [ -f "$BUILD_DIR/$app/$(echo $app | tr '[:upper:]' '[:lower:]')" ]; then
            cp "$BUILD_DIR/$app/$(echo $app | tr '[:upper:]' '[:lower:]')" "$DESTDIR/usr/bin/"
        elif [ -f "$BUILD_DIR/$app/$(echo $app | tr '[:upper:]' '[:lower:]')" ]; then
            cp "$BUILD_DIR/$app/$(echo $app | tr '[:upper:]' '[:lower:]')" "$DESTDIR/usr/bin/"
        else
            # Try python script
            cp "$THAIOS_ROOT/apps/$app/src/main.py" "$DESTDIR/usr/bin/thai-$(echo $app | sed 's/Thai//' | tr '[:upper:]' '[:lower:]')"
            chmod +x "$DESTDIR/usr/bin/thai-$(echo $app | sed 's/Thai//' | tr '[:upper:]' '[:lower:]')"
        fi
    done
    
    log "Applicazioni installate"
}

build_desktop() {
    log "Compilazione componenti ThaiDesktop..."
    
    COMPONENTS=("shell" "panel" "dock" "app-menu" "notification-center" "control-center" "widgets")
    
    for comp in "${COMPONENTS[@]}"; do
        log "  Build $comp..."
        (cd "$THAIOS_ROOT/desktop/$comp" && make BUILD_DIR="$BUILD_DIR/$comp")
        if [ -f "$BUILD_DIR/$comp/thai-$comp" ]; then
            cp "$BUILD_DIR/$comp/thai-$comp" "$DESTDIR/usr/bin/"
        fi
    done
    
    log "Componenti desktop installati"
}

build_compositor() {
    log "Compilazione ThaiDesktop compositor..."
    (cd "$THAIOS_ROOT/compositor" && make BUILD_DIR="$BUILD_DIR/compositor")
    if [ -f "$BUILD_DIR/compositor/thai-desktop" ]; then
        cp "$BUILD_DIR/compositor/thai-desktop" "$DESTDIR/usr/bin/"
    fi
    log "Compositor installato"
}

build_package_manager() {
    log "Installazione ThaiPkg..."
    (cd "$THAIOS_ROOT/thai-pkg" && make BUILD_DIR="$BUILD_DIR/thai-pkg")
    if [ -f "$BUILD_DIR/thai-pkg/thai-pkg" ]; then
        cp "$BUILD_DIR/thai-pkg/thai-pkg" "$DESTDIR/usr/bin/"
    fi
    log "ThaiPkg installato"
}

build_installer() {
    log "Installazione ThaiInstaller..."
    (cd "$THAIOS_ROOT/thai-installer" && make BUILD_DIR="$BUILD_DIR/thai-installer")
    if [ -f "$BUILD_DIR/thai-installer/thai-installer" ]; then
        cp "$BUILD_DIR/thai-installer/thai-installer" "$DESTDIR/usr/bin/"
    fi
    log "ThaiInstaller installato"
}

build_updater() {
    log "Installazione ThaiUpdater..."
    (cd "$THAIOS_ROOT/thai-updater" && make BUILD_DIR="$BUILD_DIR/thai-updater")
    if [ -f "$BUILD_DIR/thai-updater/thai-updater" ]; then
        cp "$BUILD_DIR/thai-updater/thai-updater" "$DESTDIR/usr/bin/"
    fi
    log "ThaiUpdater installato"
}

create_symlinks() {
    log "Creazione collegamenti simbolici..."
    
    # Ensure all thai-* commands exist
    for app in browser files store terminal settings media editor backup; do
        if [ ! -f "$DESTDIR/usr/bin/thai-$app" ]; then
            # Create wrapper
            cat > "$DESTDIR/usr/bin/thai-$app" << 'WRAPPER'
#!/bin/bash
SCRIPT="/usr/lib/thai-${0##*-}/main.py"
if [ -f "$SCRIPT" ]; then
    exec python3 "$SCRIPT" "$@"
else
    echo "ThaiOS: applicazione non trovata"
    exit 1
fi
WRAPPER
            chmod +x "$DESTDIR/usr/bin/thai-$app"
        fi
    done
}

generate_iso() {
    log "Generazione ISO ThaiOS..."
    ISO_DIR="$BUILD_DIR/iso"
    mkdir -p "$ISO_DIR/boot/grub"
    mkdir -p "$ISO_DIR/ThaiOS"
    
    # Copy rootfs
    cp -a "$DESTDIR"/* "$ISO_DIR/ThaiOS/"
    
    # GRUB config
    cat > "$ISO_DIR/boot/grub/grub.cfg" << 'GRUB'
set default=0
set timeout=5

loadfont unicode

set gfxmode=1920x1080
set gfxpayload=keep

insmod all_video
insmod gfxterm
insmod png

background_image /boot/splash.png

menuentry "Avvia ThaiOS" {
    linux /ThaiOS/boot/vmlinuz root=/dev/sda1 ro quiet splash
    initrd /ThaiOS/boot/initrd.img
}

menuentry "ThaiOS - Modalit\u00e0 provvisoria" {
    linux /ThaiOS/boot/vmlinuz root=/dev/sda1 ro nomodeset
    initrd /ThaiOS/boot/initrd.img
}

menuentry "Verifica memoria" {
    memtest
}
GRUB
    
    # Generate ISO
    if command -v xorriso &> /dev/null; then
        xorriso -as mkisofs \
            -iso-level 3 \
            -full-iso9660-filenames \
            -volid "ThaiOS-1.0" \
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
        log "ISO generata: $BUILD_DIR/ThaiOS-1.0.iso"
    else
        log "xorriso non trovato. Crea ISO manualmente da $ISO_DIR"
    fi
}

show_help() {
    echo "ThaiOS Build System"
    echo "==================="
    echo "Uso: $0 [comando]"
    echo ""
    echo "Comandi:"
    echo "  all              Build completo (default)"
    echo "  clean            Pulisce i file di build"
    echo "  prepare          Prepara l'ambiente"
    echo "  kernel           Configura il kernel"
    echo "  base             Installa file di base"
    echo "  branding         Installa branding"
    echo "  compositor       Build ThaiDesktop compositor"
    echo "  desktop          Build componenti desktop"
    echo "  apps             Build applicazioni"
    echo "  pkg              Build ThaiPkg"
    echo "  installer        Build ThaiInstaller"
    echo "  updater          Build ThaiUpdater"
    echo "  iso              Genera ISO"
    echo "  help             Mostra questo aiuto"
}

# Main
case "${1:-all}" in
    all)
        clean
        prepare
        build_kernel_config
        build_base
        build_branding
        build_compositor
        build_desktop
        build_apps
        build_package_manager
        build_installer
        build_updater
        create_symlinks
        generate_iso
        log "Build completata con successo!"
        ;;
    clean) clean ;;
    prepare) prepare ;;
    kernel) build_kernel_config ;;
    base) build_base ;;
    branding) build_branding ;;
    compositor) build_compositor ;;
    desktop) build_desktop ;;
    apps) build_apps ;;
    pkg) build_package_manager ;;
    installer) build_installer ;;
    updater) build_updater ;;
    iso) generate_iso ;;
    help|--help|-h) show_help ;;
    *)
        error "Comando sconosciuto: $1"
        show_help
        exit 1
        ;;
esac
