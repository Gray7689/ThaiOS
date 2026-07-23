# ThaiOS Drivers Build
# =====================

CC      ?= gcc
ASM     ?= nasm
BUILD_DIR ?= ../build
ARCH    ?= x86_64

CFLAGS  := -ffreestanding -Wall -Wextra -O2 -I../kernel/include -Iinclude
OBJDIR  := $(BUILD_DIR)/drivers

SRCS_C  := $(wildcard *.c) $(wildcard */*.c)
OBJS    := $(SRCS_C:%.c=$(OBJDIR)/%.o)

.PHONY: all clean

all: $(OBJS)
	@echo "  [DRV] Drivers built: $(words $(OBJS)) objects"

$(OBJDIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJDIR)
