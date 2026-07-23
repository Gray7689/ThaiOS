CC ?= gcc
PKGS = wayland-server wlroots xkbcommon pixman-1
CFLAGS ?= -O2 -pipe -Wall -Wextra
CFLAGS += $(shell pkg-config --cflags $(PKGS)) -Iinclude
LDFLAGS += $(shell pkg-config --libs $(PKGS)) -lm

SRC = src/main.c src/server.c src/desktop.c src/shell.c src/input.c src/output.c src/renderer.c src/config.c src/theme.c
OBJ = $(SRC:src/%.c=$(BUILD_DIR)/%.o)
TARGET = $(BUILD_DIR)/thai-desktop

.PHONY: all clean

all: $(TARGET)

$(BUILD_DIR)/%.o: src/%.c include/*.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJ)
	$(CC) $(LDFLAGS) -o $@ $^

clean:
	rm -rf $(BUILD_DIR)
