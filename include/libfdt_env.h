#ifndef _BOOT0_LIBFDT_ENV_H_
#define _BOOT0_LIBFDT_ENV_H_

#include <linux/types.h>

void *memcpy(void *dest, const void *src, size_t count);
void *memmove(void *dest, const void *src, size_t count);
int memcmp(const void *cs, const void *ct, size_t count);
void *memchr(const void *s, int c, size_t count);
char *strchr(const char *s, int c);
size_t strlen(const char *s);
size_t strnlen(const char *s, size_t count);
int strncmp(const char *cs, const char *ct, size_t count);

#ifdef __CHECKER__
#define __force __attribute__((force))
#define __bitwise __attribute__((bitwise))
#else
#define __force
#define __bitwise
#endif

typedef uint16_t __bitwise fdt16_t;
typedef uint32_t __bitwise fdt32_t;
typedef uint64_t __bitwise fdt64_t;

static inline uint16_t fdt16_to_cpu(fdt16_t value)
{
	return __builtin_bswap16((uint16_t)value);
}

static inline fdt16_t cpu_to_fdt16(uint16_t value)
{
	return (fdt16_t)__builtin_bswap16(value);
}

static inline uint32_t fdt32_to_cpu(fdt32_t value)
{
	return __builtin_bswap32((uint32_t)value);
}

static inline fdt32_t cpu_to_fdt32(uint32_t value)
{
	return (fdt32_t)__builtin_bswap32(value);
}

static inline uint64_t fdt64_to_cpu(fdt64_t value)
{
	return __builtin_bswap64((uint64_t)value);
}

static inline fdt64_t cpu_to_fdt64(uint64_t value)
{
	return (fdt64_t)__builtin_bswap64(value);
}

#endif
