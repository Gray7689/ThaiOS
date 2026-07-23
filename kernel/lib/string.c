// ThaiOS String Utilities
// =========================
// Funzioni di libreria base per il kernel.

#include <thaios.h>

void *memset(void *s, int c, usize n) {
    u8 *p = (u8*)s;
    while (n--) *p++ = (u8)c;
    return s;
}

void *memcpy(void *dest, const void *src, usize n) {
    u8 *d = (u8*)dest;
    const u8 *s = (const u8*)src;
    while (n--) *d++ = *s++;
    return dest;
}

int memcmp(const void *s1, const void *s2, usize n) {
    const u8 *p1 = (const u8*)s1;
    const u8 *p2 = (const u8*)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

void *memmove(void *dest, const void *src, usize n) {
    u8 *d = (u8*)dest;
    const u8 *s = (const u8*)src;

    if (d < s) {
        while (n--) *d++ = *s++;
    } else if (d > s) {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    }
    return dest;
}

usize strlen(const char *s) {
    usize len = 0;
    while (s[len]) len++;
    return len;
}

char *strcpy(char *dest, const char *src) {
    char *d = dest;
    while ((*d++ = *src++));
    return dest;
}

char *strncpy(char *dest, const char *src, usize n) {
    usize i;
    for (i = 0; i < n && src[i]; i++) {
        dest[i] = src[i];
    }
    for (; i < n; i++) {
        dest[i] = '\0';
    }
    return dest;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(u8*)s1 - *(u8*)s2;
}

int strncmp(const char *s1, const char *s2, usize n) {
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (n == 0) return 0;
    return *(u8*)s1 - *(u8*)s2;
}

char *strchr(const char *s, int c) {
    while (*s) {
        if (*s == (char)c) return (char*)s;
        s++;
    }
    return NULL;
}

char *strtok(char *str, const char *delim) {
    static char *next = NULL;
    if (str) next = str;

    if (!next) return NULL;

    // Salta delimitatori iniziali
    while (*next && strchr(delim, *next)) next++;
    if (!*next) return NULL;

    char *token = next;
    while (*next && !strchr(delim, *next)) next++;
    if (*next) *next++ = '\0';

    return token;
}
