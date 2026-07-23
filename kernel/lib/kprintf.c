// ThaiOS Kernel Printf
// ======================
// Implementazione minimale di printf per kernel debug.
// Output via seriale (COM1) e framebuffer.

#include <thaios.h>
#include <stdarg.h>

#define COM1_PORT 0x3F8

static void serial_write_char(char c) {
    // Attendi che il buffer seriale sia libero
    while (!(__inbyte(COM1_PORT + 5) & 0x20));
    __outbyte(COM1_PORT, c);
}

static void serial_write_string(const char *str) {
    while (*str) {
        if (*str == '\n') serial_write_char('\r');
        serial_write_char(*str++);
    }
}

static void print_num(u64 num, int base, bool sign) {
    char buf[65];
    char digits[] = "0123456789abcdef";
    int i = 0;
    bool neg = false;

    if (sign && (i64)num < 0) {
        neg = true;
        num = -(i64)num;
    }

    if (num == 0) {
        buf[i++] = '0';
    } else {
        while (num > 0) {
            buf[i++] = digits[num % base];
            num /= base;
        }
    }

    if (neg) buf[i++] = '-';

    while (i > 0) serial_write_char(buf[--i]);
}

void kprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            serial_write_char(*fmt++);
            continue;
        }

        fmt++;  // Skip '%'

        bool left_align = false;
        bool force_sign = false;
        bool alt_form = false;
        char pad = ' ';

        // Flags
        while (1) {
            if (*fmt == '-') { left_align = true; fmt++; }
            else if (*fmt == '+') { force_sign = true; fmt++; }
            else if (*fmt == '#') { alt_form = true; fmt++; }
            else if (*fmt == '0') { pad = '0'; fmt++; }
            else break;
        }

        // Width
        int width = 0;
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt++ - '0');
        }

        // Length (ignoriamo per semplicità)
        if (*fmt == 'l') { fmt++; }
        if (*fmt == 'l') { fmt++; }

        // Specifier
        switch (*fmt) {
            case 'd':
            case 'i': print_num(va_arg(args, i64), 10, true); break;
            case 'u': print_num(va_arg(args, u64), 10, false); break;
            case 'x': print_num(va_arg(args, u64), 16, false); break;
            case 'X': print_num(va_arg(args, u64), 16, false); break;
            case 'p': serial_write_string("0x"); print_num(va_arg(args, u64), 16, false); break;
            case 's': serial_write_string(va_arg(args, const char*)); break;
            case 'c': serial_write_char((char)va_arg(args, int)); break;
            case '%': serial_write_char('%'); break;
            case 'n': break;
            default: serial_write_char('%'); serial_write_char(*fmt); break;
        }

        fmt++;
    }

    va_end(args);
}

void panic(const char *msg) {
    kprintf("\n\n*** KERNEL PANIC ***\n");
    kprintf("Reason: %s\n", msg);
    kprintf("System halted.\n");

    __asm__ volatile("cli; hlt");
    while (1) { __asm__ volatile("hlt"); }
}

void kassert(bool cond, const char *msg) {
    if (!cond) {
        panic(msg);
    }
}
