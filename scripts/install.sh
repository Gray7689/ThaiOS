#!/bin/bash
#
# ThaiOS Installation Script
#

set -e

PREFIX="${1:-/usr}"
DESTDIR="${2:-}"
THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$THAIOS_ROOT/build"

log() { echo -e "\033[1;32m[ThaiOS]\033[0m $*"; }

install_file() {
    local src="$1"
    local dst="${DESTDIR}$2"
    mkdir -p "$(dirname "$dst")"
    cp "$src" "$dst"
    log "Installato: $2"
}

install_branding() {
    log "Installazione branding..."
    
    # Logo
    for f in "$THAIOS_ROOT/branding/logo/"*; do
        install_file "$f" "/usr/share/ThaiOS/branding/logo/$(basename "$f")"
    done
    
    # Wallpapers
    for f in "$THAIOS_ROOT/branding/wallpapers/"*; do
        install_file "$f" "/usr/share/ThaiOS/wallpapers/$(basename "$f")"
    done
    
    # Themes
    cp -r "$THAIOS_ROOT/branding/themes/"* "${DESTDIR}/usr/share/themes/"
    
    # Icons
    cp -r "$THAIOS_ROOT/branding/icons/"* "${DESTDIR}/usr/share/icons/"
}

install_base() {
    log "Installazione file di base..."
    
    install_file "$THAIOS_ROOT/base/etc/os-release" "/etc/os-release"
    install_file "$THAIOS_ROOT/base/etc/issue" "/etc/issue"
    install_file "$THAIOS_ROOT/base/etc/lsb-release" "/etc/lsb-release"
    install_file "$THAIOS_ROOT/base/etc/motd" "/etc/motd"
    install_file "$THAIOS_ROOT/base/etc/profile" "/etc/profile"
    install_file "$THAIOS_ROOT/base/etc/bash.bashrc" "/etc/bash.bashrc"
    install_file "$THAIOS_ROOT/base/etc/zshrc" "/etc/zshrc"
    install_file "$THAIOS_ROOT/base/usr/share/ThaiOS/session.conf" "/usr/share/ThaiOS/session.conf"
}

install_apps() {
    log "Installazione applicazioni..."
    
    APPS_DIR="$THAIOS_ROOT/apps"
    for app_dir in "$APPS_DIR"/*/; do
        app_name=$(basename "$app_dir")
        binary_name="thai-$(echo "$app_name" | sed 's/Thai//' | tr '[:upper:]' '[:lower:]')"
        
        if [ -f "$BUILD_DIR/$app_name/$binary_name" ]; then
            install_file "$BUILD_DIR/$app_name/$binary_name" "/usr/bin/$binary_name"
        elif [ -f "$app_dir/src/main.py" ]; then
            install_file "$app_dir/src/main.py" "/usr/bin/$binary_name"
            chmod +x "${DESTDIR}/usr/bin/$binary_name"
        fi
    done
}

install_desktop() {
    log "Installazione componenti desktop..."
    
    COMPONENTS="shell panel dock app-menu notification-center control-center widgets"
    for comp in $COMPONENTS; do
        if [ -f "$BUILD_DIR/$comp/thai-$comp" ]; then
            install_file "$BUILD_DIR/$comp/thai-$comp" "/usr/bin/thai-$comp"
        elif [ -f "$THAIOS_ROOT/desktop/$comp/main.py" ]; then
            install_file "$THAIOS_ROOT/desktop/$comp/main.py" "/usr/bin/thai-$comp"
            chmod +x "${DESTDIR}/usr/bin/thai-$comp"
        fi
    done
    
    # Compositor
    if [ -f "$BUILD_DIR/compositor/thai-desktop" ]; then
        install_file "$BUILD_DIR/compositor/thai-desktop" "/usr/bin/thai-desktop"
    fi
}

install_pkgs() {
    log "Installazione ThaiPkg..."
    if [ -f "$BUILD_DIR/thai-pkg/thai-pkg" ]; then
        install_file "$BUILD_DIR/thai-pkg/thai-pkg" "/usr/bin/thai-pkg"
    fi
    install_file "$THAIOS_ROOT/thai-pkg/src/thai-pkg.py" "/usr/lib/thai-pkg/src/thai_pkg.py"
}

install_installer() {
    log "Installazione ThaiInstaller..."
    if [ -f "$BUILD_DIR/thai-installer/thai-installer" ]; then
        install_file "$BUILD_DIR/thai-installer/thai-installer" "/usr/bin/thai-installer"
    fi
}

install_updater() {
    log "Installazione ThaiUpdater..."
    if [ -f "$BUILD_DIR/thai-updater/thai-updater" ]; then
        install_file "$BUILD_DIR/thai-updater/thai-updater" "/usr/bin/thai-updater"
    fi
}

log "Inizio installazione ThaiOS..."
install_base
install_branding
install_desktop
install_apps
install_pkgs
install_installer
install_updater
log "Installazione ThaiOS completata!"
