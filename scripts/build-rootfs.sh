#!/bin/bash
#
# ThaiOS Rootfs Builder
# Creates a complete ThaiOS root filesystem from scratch
#

set -e

THAIOS_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$THAIOS_ROOT/build"
ROOTFS_DIR="$BUILD_DIR/rootfs"
ISO_DIR="$BUILD_DIR/iso"
ROOTFS_SQUASH="$BUILD_DIR/filesystem.squashfs"
THAIOS_VERSION="1.0"
THAIOS_CODENAME="Songkran"
DEBIAN_SUITE="bookworm"
DEBIAN_MIRROR="http://deb.debian.org/debian"
# Mirror alternativo se il default fallisce
DEBIAN_MIRROR_FALLBACK="http://ftp.us.debian.org/debian"

# Packages da includere direttamente nel bootstrap
INCLUDE_PKGS="systemd,systemd-sysv,dbus,udev,linux-image-amd64,firmware-linux,"
INCLUDE_PKGS+="xserver-xorg-core,xserver-xorg-input-all,xserver-xorg-video-all,xinit,"
INCLUDE_PKGS+="python3,python3-gi,python3-gi-cairo,"
INCLUDE_PKGS+="libgtk-3-0,gir1.2-gtk-3-0,"
INCLUDE_PKGS+="network-manager,alsa-utils,pulseaudio,"
INCLUDE_PKGS+="zsh,bash-completion,sudo,curl,wget,nano,htop,ca-certificates,"
INCLUDE_PKGS+="fonts-dejavu,fonts-noto,desktop-file-utils"

log() { echo -e "\033[1;32m[ThaiOS]\033[0m $*"; }
error() { echo -e "\033[1;31m[ERROR]\033[0m $*"; exit 1; }

clean_rootfs() {
    log "Pulizia rootfs precedente..."
    rm -rf "$ROOTFS_DIR" "$ROOTFS_SQUASH"
    mkdir -p "$ROOTFS_DIR"
}

bootstrap() {
    log "Fase 1: Bootstrap del sistema base (Debian $DEBIAN_SUITE)..."
    
    if ! command -v debootstrap &>/dev/null; then
        log "Installazione debootstrap..."
        apt-get update && apt-get install -y debootstrap
    fi
    
    debootstrap --arch=amd64 --variant=minbase \
        --include="$INCLUDE_PKGS" \
        "$DEBIAN_SUITE" "$ROOTFS_DIR" "$DEBIAN_MIRROR"
    
    log "Bootstrap completato"
}

configure_system() {
    log "Fase 2: Configurazione sistema ThaiOS..."
    
    # Mount special filesystems
    mount -t proc none "$ROOTFS_DIR/proc" 2>/dev/null || true
    mount -t sysfs none "$ROOTFS_DIR/sys" 2>/dev/null || true
    mount -o bind /dev "$ROOTFS_DIR/dev" 2>/dev/null || true
    mount -o bind /dev/pts "$ROOTFS_DIR/dev/pts" 2>/dev/null || true
    
    # Enable networking inside chroot
    cp /etc/resolv.conf "$ROOTFS_DIR/etc/resolv.conf" 2>/dev/null || echo "nameserver 8.8.8.8" > "$ROOTFS_DIR/etc/resolv.conf"
    
    # ThaiOS identity files
    mkdir -p "$ROOTFS_DIR/etc"
    
    cat > "$ROOTFS_DIR/etc/os-release" << 'OSRELEASE'
ID=thaios
NAME="ThaiOS"
PRETTY_NAME="ThaiOS 1.0 (Songkran)"
VERSION_ID="1.0"
VERSION="1.0 (Songkran)"
VERSION_CODENAME=songkran
BUILD_ID=20250723
ANSI_COLOR="0;31;44"
HOME_URL="https://thaios.dev"
SUPPORT_URL="https://thaios.dev/support"
BUG_REPORT_URL="https://thaios.dev/issues"
LOGO=thaios-logo
OSRELEASE
    
    cat > "$ROOTFS_DIR/etc/lsb-release" << 'LSB'
DISTRIB_ID=ThaiOS
DISTRIB_RELEASE=1.0
DISTRIB_CODENAME=songkran
DISTRIB_DESCRIPTION="ThaiOS 1.0 (Songkran)"
LSB
    
    echo "ThaiOS 1.0 \n \l" > "$ROOTFS_DIR/etc/issue"
    
    # Setup hostname
    echo "ThaiOS" > "$ROOTFS_DIR/etc/hostname"
    echo "127.0.1.1 ThaiOS" >> "$ROOTFS_DIR/etc/hosts"
    
    # Configure apt for ThaiOS
    cat > "$ROOTFS_DIR/etc/apt/sources.list" << 'APTSOURCES'
deb http://deb.debian.org/debian bookworm main contrib non-free-firmware
deb http://deb.debian.org/debian bookworm-updates main contrib non-free-firmware
deb http://security.debian.org/debian-security bookworm-security main contrib non-free-firmware
APTSOURCES
    apt-get clean --root="$ROOTFS_DIR" 2>/dev/null || true
    rm -rf "$ROOTFS_DIR/var/lib/apt/lists/"* 2>/dev/null || true

    log "Pacchetti ThaiOS installati"
}

install_thaios_branding() {
    log "Fase 3: Installazione branding ThaiOS..."
    
    # Remove Debian/Ubuntu branding from GRUB
    rm -f "$ROOTFS_DIR/etc/default/grub.d/"* 2>/dev/null || true
    
    mkdir -p "$ROOTFS_DIR/usr/share/ThaiOS"
    
    # Copy branding assets
    cp -r "$THAIOS_ROOT/branding/logo" "$ROOTFS_DIR/usr/share/ThaiOS/"
    cp -r "$THAIOS_ROOT/branding/wallpapers" "$ROOTFS_DIR/usr/share/ThaiOS/"
    
    # Install themes
    mkdir -p "$ROOTFS_DIR/usr/share/themes"
    cp -r "$THAIOS_ROOT/branding/themes/"* "$ROOTFS_DIR/usr/share/themes/"
    
    # Install icons
    if [ -d "$THAIOS_ROOT/branding/icons/ThaiOS-Icons" ]; then
        mkdir -p "$ROOTFS_DIR/usr/share/icons"
        cp -r "$THAIOS_ROOT/branding/icons/ThaiOS-Icons" "$ROOTFS_DIR/usr/share/icons/"
    fi
    
    # Font config
    mkdir -p "$ROOTFS_DIR/usr/share/fonts/truetype/thaios"
    if [ -d "$THAIOS_ROOT/branding/fonts" ]; then
        cp "$THAIOS_ROOT/branding/fonts/"* "$ROOTFS_DIR/usr/share/fonts/truetype/thaios/" 2>/dev/null || true
    fi
    fc-cache -f "$ROOTFS_DIR/usr/share/fonts" 2>/dev/null || true
    
    # GTK settings for ThaiOS theme
    mkdir -p "$ROOTFS_DIR/etc/gtk-3.0"
    cat > "$ROOTFS_DIR/etc/gtk-3.0/settings.ini" << 'GTK'
[Settings]
gtk-theme-name=ThaiOS-Dark
gtk-icon-theme-name=ThaiOS-Icons
gtk-font-name=Sarabun 11
gtk-cursor-theme-name=ThaiOS-Cursors
gtk-enable-animations=1
gtk-application-prefer-dark-theme=1
GTK
    
    log "Branding installato"
}

install_thaios_apps() {
    log "Fase 4: Installazione applicazioni ThaiOS..."
    
    APPS_DIR="$ROOTFS_DIR/usr/lib/ThaiOS"
    BIN_DIR="$ROOTFS_DIR/usr/bin"
    mkdir -p "$APPS_DIR" "$BIN_DIR"
    
    local apps=("ThaiBrowser" "ThaiFiles" "ThaiStore" "ThaiTerminal" 
                "ThaiSettings" "ThaiMedia" "ThaiEditor" "ThaiBackup")
    
    for app in "${apps[@]}"; do
        local src="$THAIOS_ROOT/apps/$app/src/main.py"
        local bin_name="thai-$(echo "$app" | sed 's/Thai//' | tr '[:upper:]' '[:lower:]')"
        
        if [ -f "$src" ]; then
            mkdir -p "$APPS_DIR/$app"
            cp "$src" "$APPS_DIR/$app/main.py"
            
            cat > "$BIN_DIR/$bin_name" << BIN
#!/bin/bash
exec python3 /usr/lib/ThaiOS/$app/main.py "\$@"
BIN
            chmod +x "$BIN_DIR/$bin_name"
        fi
    done
    
    # ThaiPkg
    mkdir -p "$APPS_DIR/thai-pkg"
    cp "$THAIOS_ROOT/thai-pkg/src/thai-pkg.py" "$APPS_DIR/thai-pkg/"
    cat > "$BIN_DIR/thai-pkg" << 'PKG'
#!/bin/bash
exec python3 /usr/lib/ThaiOS/thai-pkg/thai-pkg.py "$@"
PKG
    chmod +x "$BIN_DIR/thai-pkg"
    
    # ThaiInstaller
    mkdir -p "$APPS_DIR/thai-installer"
    cp "$THAIOS_ROOT/thai-installer/src/thai-installer.py" "$APPS_DIR/thai-installer/"
    cat > "$BIN_DIR/thai-installer" << 'INST'
#!/bin/bash
exec python3 /usr/lib/ThaiOS/thai-installer/thai-installer.py "$@"
INST
    chmod +x "$BIN_DIR/thai-installer"
    
    # ThaiUpdater
    mkdir -p "$APPS_DIR/thai-updater"
    cp "$THAIOS_ROOT/thai-updater/src/thai-updater.py" "$APPS_DIR/thai-updater/"
    cat > "$BIN_DIR/thai-updater" << 'UPD'
#!/bin/bash
exec python3 /usr/lib/ThaiOS/thai-updater/thai-updater.py "$@"
UPD
    chmod +x "$BIN_DIR/thai-updater"
    
    log "Applicazioni installate"
}

install_desktop_components() {
    log "Fase 5: Installazione componenti ThaiDesktop..."
    
    BIN_DIR="$ROOTFS_DIR/usr/bin"
    
    # ThaiPanel
    if [ -f "$THAIOS_ROOT/desktop/panel/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/panel/main.py" "$BIN_DIR/thai-panel"
        chmod +x "$BIN_DIR/thai-panel"
    fi
    
    # ThaiDock
    if [ -f "$THAIOS_ROOT/desktop/dock/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/dock/main.py" "$BIN_DIR/thai-dock"
        chmod +x "$BIN_DIR/thai-dock"
    fi
    
    # ThaiAppMenu
    if [ -f "$THAIOS_ROOT/desktop/app-menu/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/app-menu/main.py" "$BIN_DIR/thai-app-menu"
        chmod +x "$BIN_DIR/thai-app-menu"
    fi
    
    # ThaiNotifications
    if [ -f "$THAIOS_ROOT/desktop/notification-center/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/notification-center/main.py" "$BIN_DIR/thai-notifications"
        chmod +x "$BIN_DIR/thai-notifications"
    fi
    
    # ThaiControlCenter
    if [ -f "$THAIOS_ROOT/desktop/control-center/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/control-center/main.py" "$BIN_DIR/thai-control-center"
        chmod +x "$BIN_DIR/thai-control-center"
    fi
    
    # ThaiWidgets
    if [ -f "$THAIOS_ROOT/desktop/widgets/main.py" ]; then
        cp "$THAIOS_ROOT/desktop/widgets/main.py" "$BIN_DIR/thai-widgets"
        chmod +x "$BIN_DIR/thai-widgets"
    fi
    
    # Desktop entries (.desktop files for app menu)
    mkdir -p "$ROOTFS_DIR/usr/share/applications"
    
    for app in Browser Files Store Terminal Settings Media Editor Backup; do
        local low="$(echo "$app" | tr '[:upper:]' '[:lower:]')"
        cat > "$ROOTFS_DIR/usr/share/applications/thai-$low.desktop" << DESKTOP
[Desktop Entry]
Name=Thai$app
Comment=$app di ThaiOS
Exec=thai-$low
Icon=Thai$app
Terminal=false
Type=Application
Categories=ThaiOS;
DesktopName=ThaiOS
DESKTOP
    done
    
    # Autostart ThaiDesktop
    mkdir -p "$ROOTFS_DIR/etc/xdg/autostart"
    cat > "$ROOTFS_DIR/etc/xdg/autostart/thai-desktop.desktop" << 'AUTOSTART'
[Desktop Entry]
Type=Application
Name=ThaiDesktop
Comment=ThaiOS Desktop Environment
Exec=thai-panel
OnlyShowIn=ThaiOS
X-ThaiOS-Autostart=true
AUTOSTART
    
    log "Componenti desktop installati"
}

configure_display_manager() {
    log "Fase 6: Configurazione display manager..."
    
    # Use XDM or lightdm as login manager with ThaiOS branding
    apt-get install -y --no-install-recommends --root="$ROOTFS_DIR" lightdm lightdm-gtk-greeter 2>/dev/null || true
    
    # LightDM with ThaiOS theme
    mkdir -p "$ROOTFS_DIR/etc/lightdm"
    
    cat > "$ROOTFS_DIR/etc/lightdm/lightdm.conf" << 'LIGHTDM'
[Seat:*]
greeter-session=lightdm-gtk-greeter
user-session=thaios
greeter-hide-users=false
greeter-show-manual-login=false
background=/usr/share/ThaiOS/wallpapers/ThaiOS-day.png
LIGHTDM
    
    cat > "$ROOTFS_DIR/etc/lightdm/lightdm-gtk-greeter.conf" << 'GREETER'
[greeter]
background=/usr/share/ThaiOS/wallpapers/ThaiOS-day.png
theme-name=ThaiOS-Dark
icon-theme-name=ThaiOS-Icons
font-name=Sarabun 11
xft-antialias=true
xft-hintstyle=hintslight
default-user-image=#ThaiOS
GREETER
    
    log "Display manager configurato"
}

configure_boot_splash() {
    log "Fase 7: Configurazione boot splash ThaiOS..."
    
    apt-get install -y --no-install-recommends --root="$ROOTFS_DIR" plymouth plymouth-themes 2>/dev/null || true
    
    # ThaiOS Plymouth theme
    mkdir -p "$ROOTFS_DIR/usr/share/plymouth/themes/ThaiOS"
    
    cat > "$ROOTFS_DIR/usr/share/plymouth/themes/ThaiOS/ThaiOS.plymouth" << PLYMOUTH
[Plymouth Theme]
Name=ThaiOS
Description=ThaiOS Boot Splash
ModuleName=script
PLYMOUTH
    
    cat > "$ROOTFS_DIR/usr/share/plymouth/themes/ThaiOS/ThaiOS.script" << SCRIPT
wallpaper_image = Image("splash.png");
wallpaper_sprite = Sprite();
wallpaper_sprite.SetImage(wallpaper_image);
wallpaper_sprite.SetZ(-100);
fun message_callback (text) { }
fun question_callback (text, length) { return ""; }
fun display_question_callback (text, entry_buffer, entry_cursor, bullet) { }
fun display_normal_callback (text) { }
fun display_password_callback (text, length, bullet) { }
fun keyboard_input_callback (key) { }
fun refresh_callback () { }
fun root_callback () { }
SCRIPT
    
    # Set as default
    ln -sf /usr/share/plymouth/themes/ThaiOS/ThaiOS.plymouth \
        "$ROOTFS_DIR/etc/alternatives/default.plymouth" 2>/dev/null || true
    
    # Update initramfs (skip errors, handled on first boot)
    chroot "$ROOTFS_DIR" update-initramfs -u 2>/dev/null || true
    rm -f "$ROOTFS_DIR/etc/machine-id" 2>/dev/null || true
    
    log "Boot splash configurato"
}

create_user_environment() {
    log "Fase 8: Creazione ambiente utente..."
    
    # Skel files for new users
    mkdir -p "$ROOTFS_DIR/etc/skel"
    mkdir -p "$ROOTFS_DIR/etc/skel/.config/ThaiOS"
    mkdir -p "$ROOTFS_DIR/etc/skel/Desktop"
    mkdir -p "$ROOTFS_DIR/etc/skel/Documents"
    mkdir -p "$ROOTFS_DIR/etc/skel/Downloads"
    
    # ThaiOS wallpaper on desktop
    cat > "$ROOTFS_DIR/etc/skel/.config/ThaiOS/settings.conf" << 'CONF'
[Desktop]
theme=ThaiOS-Dark
icons=ThaiOS-Icons
wallpaper=/usr/share/ThaiOS/wallpapers/ThaiOS-day.png
font=Sarabun 11
CONF
    
    # Shell configuration
    cat > "$ROOTFS_DIR/etc/skel/.bashrc" << 'BASHRC'
export THAIOS_VERSION="1.0"
PS1='\[\e[38;5;196m\]┌[\[\e[0m\]\u\[\e[38;5;32m\]@\[\e[0m\]\h\[\e[38;5;196m\]]─[\[\e[0m\]\w\[\e[38;5;196m\]]
└[\[\e[0m\]\t\[\e[38;5;196m\]]>\[\e[0m\] '
alias ls='ls --color=auto'
alias ll='ls -la'
alias thai-update='sudo thai-pkg update'
alias thai-install='sudo thai-pkg install'
alias thai-remove='sudo thai-pkg remove'
BASHRC
    
    # Default wallpaper for desktop
    if [ -f "$THAIOS_ROOT/branding/wallpapers/default-thaios.xml" ]; then
        mkdir -p "$ROOTFS_DIR/usr/share/backgrounds/ThaiOS"
        cp "$THAIOS_ROOT/branding/wallpapers/"* "$ROOTFS_DIR/usr/share/backgrounds/ThaiOS/" 2>/dev/null || true
    fi
    
    log "Ambiente utente creato"
}

finalize() {
    log "Fase 9: Finalizzazione..."
    
    # Clean apt
    apt-get clean --root="$ROOTFS_DIR" 2>/dev/null || true
    
    # Remove temporary files
    rm -rf "$ROOTFS_DIR/tmp/"*
    rm -rf "$ROOTFS_DIR/var/log/"*
    rm -rf "$ROOTFS_DIR/var/cache/apt/"*
    
    # Remove machine ID (will be generated on first boot)
    rm -f "$ROOTFS_DIR/etc/machine-id"
    rm -f "$ROOTFS_DIR/var/lib/dbus/machine-id"
    
    # Unmount special filesystems
    umount -l "$ROOTFS_DIR/dev/pts" 2>/dev/null || true
    umount -l "$ROOTFS_DIR/proc" 2>/dev/null || true
    umount -l "$ROOTFS_DIR/sys" 2>/dev/null || true
    umount -l "$ROOTFS_DIR/dev" 2>/dev/null || true
    
    log "Rootfs finalizzato"
}

# Main
case "${1:-all}" in
    all)
        clean_rootfs
        bootstrap
        configure_system
        install_thaios_branding
        install_thaios_apps
        install_desktop_components
        configure_display_manager
        configure_boot_splash
        create_user_environment
        finalize
        log "Rootfs ThaiOS creato in: $ROOTFS_DIR"
        ;;
    clean) clean_rootfs ;;
    bootstrap) bootstrap ;;
    configure) configure_system ;;
    branding) install_thaios_branding ;;
    apps) install_thaios_apps ;;
    desktop) install_desktop_components ;;
    display-manager) configure_display_manager ;;
    splash) configure_boot_splash ;;
    user) create_user_environment ;;
    finalize) finalize ;;
    *)
        echo "Uso: $0 {all|clean|bootstrap|configure|branding|apps|desktop|display-manager|splash|user|finalize}"
        ;;
esac
