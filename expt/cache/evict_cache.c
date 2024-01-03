#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <memory.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include "cacheutils.h"

void evict(char *mem, char *mapping, int level)
{
    // visit mem驱逐代码
    // Intel(R) Core(TM) i7-6700 CPU @ 3.40GHz
    switch (level)
    {
    case 0:
    case 1:
        break;
    case 2:
        // cache to l2 by mmap
        for (uint64_t j = 0; j < 11; j += 1)
        {
            maccess(&mem[j * 4096]);
        }
        break;
    case 3:
        // cache to l3 by mmap
        for (uint64_t j = 0; j < 320; j++)
        {
            maccess(&mem[j * 4096]);
        }
        break;
    default:
        // cache to mem
        flush(mapping);
        break;
    }
}

void test_cache_visited(int level, int rdtsc_base_time)
{
    char *mapping = (char *)mmap(0, 12 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    char *mem = (char *)mmap(0, 320 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

    uint32_t count = 8000000;
    uint64_t sum[] = {0, 0, 0, 0}, pmu_count[] = {0, 0, 0, 0}, visit_times = 0;
    uint64_t sum_time_stamp = 0;

    for (uint32_t i = 0; i < count; i++)
    {
        maccess(mapping);
        evict(mem, mapping, level);

        if (i < count / 2)
        {
            asm volatile(
                "mfence\n\t"
                "mov $4, %%ecx\n\t"
                "read_pmc:"
                "dec %%ecx\n\t"
                "rdpmc\n\t"
                "pushq %%rax\n\t"
                "jne read_pmc\n\t"
                "mfence\n\t"

                // // 访问内存地址
                "movq (%%rbx), %%rax\n\t"

                "mfence\n\t"
                "mov $0, %%ecx\n\t"
                "rdpmc\n\t"
                "popq %%rbx\n\t"
                "sub %%ebx, %%eax\n\t"
                "movl %%eax, %%ebx\n\t"
                "movq %%rbx, %0\n\t"

                "mov $1, %%ecx\n\t"
                "rdpmc\n\t"
                "popq %%rbx\n\t"
                "sub %%ebx, %%eax\n\t"
                "movl %%eax, %%ebx\n\t"
                "movq %%rbx, %1\n\t"

                "mov $2, %%ecx\n\t"
                "rdpmc\n\t"
                "popq %%rbx\n\t"
                "sub %%ebx, %%eax\n\t"
                "movl %%eax, %%ebx\n\t"
                "movq %%rbx, %2\n\t"

                "mov $3, %%ecx\n\t"
                "rdpmc\n\t"
                "popq %%rbx\n\t"
                "sub %%ebx, %%eax\n\t"
                "movl %%eax, %%ebx\n\t"
                "movq %%rbx, %3\n\t"

                : "=m"(pmu_count[0]), "=m"(pmu_count[1]),
                  "=m"(pmu_count[2]), "=m"(pmu_count[3]) // outputs
                : "b"(mapping)                           // inputs
            );
            visit_times++;
            for (uint64_t j = 0; j < 4; j++)
            {
                sum[j] += pmu_count[j];
            }
        }
        else
        {
            uint64_t time_stamp_start, time_stamp_end;
            time_stamp_start = rdtsc();
            asm volatile("movq (%0),%%rax"
                         :
                         : "c"(mapping)
                         : "rax");
            time_stamp_end = rdtsc();
            sum_time_stamp += (time_stamp_end - time_stamp_start);
        }
    }
    printf("| %2d | %7ld | %7ld | %7ld | %7ld | %7ld | %7ld ",
           level, visit_times, sum[0], sum[1], sum[2], sum[3],
           sum[0] + sum[1] + sum[2] + sum[3]);
    sum_time_stamp -= (visit_times * rdtsc_base_time);
    printf("| %10ld | %10ld |\n", sum_time_stamp, sum_time_stamp / (count / 2));
}

int main(void)
{
    uint64_t count = 99999999, rdtsc_base_time = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        uint64_t time_stamp_start, time_stamp_end;
        time_stamp_start = rdtsc();
        time_stamp_end = rdtsc();
        rdtsc_base_time += (time_stamp_end - time_stamp_start);
    }
    rdtsc_base_time /= count;
    printf("rdtsc base time:%ld\n", rdtsc_base_time);
    printf("| id | maccess |  l1 hit |  l2 hit |  l3 hit |  FB hit | all hit | sum clocks | avg clocks |\n");
    for (int i = 1; i < 5; i++)
    {
        test_cache_visited(i, rdtsc_base_time);
    }
    return 0;
}
