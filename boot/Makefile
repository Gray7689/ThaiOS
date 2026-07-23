# ThaiOS Bootloader Build
# =======================

LIMINE_DIR ?= ../boot/limine
BUILD_DIR  ?= ../build

.PHONY: all clean

all:
	@echo "  [BOOT] Limine configuration ready"
	@mkdir -p $(BUILD_DIR)

	# Copia i binari Limine precompilati (scaricati separatamente)
	@if [ -f $(LIMINE_DIR)/limine.sys ]; then \
		cp $(LIMINE_DIR)/limine.sys $(BUILD_DIR)/; \
	fi

clean:
	@echo "  [BOOT] Clean"
	rm -f $(BUILD_DIR)/limine.sys
