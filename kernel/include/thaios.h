#ifndef _THAIOS_H
#define _THAIOS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define THAIOS_VERSION_MAJOR 0
#define THAIOS_VERSION_MINOR 1
#define THAIOS_VERSION_PATCH 0

#define NULL ((void*)0)
#define ALIGN_UP(x, a) (((x) + (a) - 1) & ~((a) - 1))
#define ALIGN_DOWN(x, a) ((x) & ~((a) - 1))
#define PAGE_SIZE 4096
#define PAGE_ALIGN(x) ALIGN_UP(x, PAGE_SIZE)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef uint64_t usize;
typedef int64_t  isize;

typedef u64 paddr_t;
typedef u64 vaddr_t;

enum { SUCCESS = 0, EFAIL = -1, ENOMEM = -2, EINVAL = -3, ENOTSUP = -4 };

void panic(const char *msg) __attribute__((noreturn));
void kprintf(const char *fmt, ...);
void kassert(bool cond, const char *msg);

#endif
