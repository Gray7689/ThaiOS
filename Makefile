.PHONY: all clean install world kernel compositor desktop apps pkg installer updater iso

PREFIX ?= /usr
DESTDIR ?=
BUILD_DIR ?= build

all: kernel compositor desktop apps pkg installer updater

world: all iso

# ============================================================
# Kernel
# ============================================================
kernel:
	@echo "[ThaiOS] Building kernel..."
	@mkdir -p $(BUILD_DIR)/kernel
	cp kernel/config $(BUILD_DIR)/kernel/.config
	make -C $(BUILD_DIR)/kernel -j$$(nproc) bzImage
	@echo "[ThaiOS] Kernel built successfully"

# ============================================================
# ThaiDesktop Compositor
# ============================================================
compositor:
	@echo "[ThaiOS] Building ThaiDesktop compositor..."
	$(MAKE) -C compositor BUILD_DIR=$(abspath $(BUILD_DIR))/compositor

# ============================================================
# Desktop components
# ============================================================
desktop:
	@echo "[ThaiOS] Building desktop components..."
	$(MAKE) -C desktop/panel BUILD_DIR=$(abspath $(BUILD_DIR))/panel
	$(MAKE) -C desktop/dock BUILD_DIR=$(abspath $(BUILD_DIR))/dock
	$(MAKE) -C desktop/app-menu BUILD_DIR=$(abspath $(BUILD_DIR))/app-menu
	$(MAKE) -C desktop/notification-center BUILD_DIR=$(abspath $(BUILD_DIR))/notification-center
	$(MAKE) -C desktop/control-center BUILD_DIR=$(abspath $(BUILD_DIR))/control-center
	$(MAKE) -C desktop/widgets BUILD_DIR=$(abspath $(BUILD_DIR))/widgets
	$(MAKE) -C desktop/shell BUILD_DIR=$(abspath $(BUILD_DIR))/shell

# ============================================================
# Applications
# ============================================================
apps:
	@echo "[ThaiOS] Building applications..."
	$(MAKE) -C apps/ThaiBrowser BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiBrowser
	$(MAKE) -C apps/ThaiFiles BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiFiles
	$(MAKE) -C apps/ThaiStore BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiStore
	$(MAKE) -C apps/ThaiTerminal BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiTerminal
	$(MAKE) -C apps/ThaiSettings BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiSettings
	$(MAKE) -C apps/ThaiMedia BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiMedia
	$(MAKE) -C apps/ThaiEditor BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiEditor
	$(MAKE) -C apps/ThaiBackup BUILD_DIR=$(abspath $(BUILD_DIR))/ThaiBackup

# ============================================================
# Package Manager
# ============================================================
pkg:
	@echo "[ThaiOS] Building ThaiPkg..."
	$(MAKE) -C thai-pkg BUILD_DIR=$(abspath $(BUILD_DIR))/thai-pkg

# ============================================================
# Installer
# ============================================================
installer:
	@echo "[ThaiOS] Building ThaiInstaller..."
	$(MAKE) -C thai-installer BUILD_DIR=$(abspath $(BUILD_DIR))/thai-installer

# ============================================================
# Updater
# ============================================================
updater:
	@echo "[ThaiOS] Building ThaiUpdater..."
	$(MAKE) -C thai-updater BUILD_DIR=$(abspath $(BUILD_DIR))/thai-updater

# ============================================================
# ISO Generation
# ============================================================
iso:
	@echo "[ThaiOS] Generating ISO image..."
	@bash scripts/build-iso.sh $(BUILD_DIR)

# ============================================================
# Installation
# ============================================================
install:
	@echo "[ThaiOS] Installing system..."
	@bash scripts/install.sh $(PREFIX) $(DESTDIR)

# ============================================================
# Cleanup
# ============================================================
clean:
	rm -rf $(BUILD_DIR)

distclean: clean
	rm -rf deps
