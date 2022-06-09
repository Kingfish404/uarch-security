#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <memory.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include "cacheutils.h"

uint64_t read_pmu(char *p)
{
    uint32_t lo, hi, pctr_arg;
    pctr_arg = 0x0; // read first fixed-function counter
    uint64_t pmu_count;

    maccess(p);

    char *attacker_map = (char *)mmap(0, 12 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

    // flush(p);
    // 此处构造驱逐代码
    // for (int j = 1; j < 12; j++)
    // {
    //     maccess(attacker_map + j * 4096);
    // }
    //---- set index tag VIPT 6-12 index, 0-5 cache line ,tag physical address
    //----
    //----
    //----
    //----

    // 12*64B

    // maccess (p)
    // maccess(evcit set)
    // maccess(p) time l1 miss l1 hit // pmu calulate l1 hit

    // cache line 64B
    // L1D 32KB
    // 512 cache line
    //
    // uint64_t begin = rdtsc();
    // maccess(p);
    // if(*p != 'a') *p = 'a';
    // else *p = 'b';

    // for(int j=1;j<12;j++){
    // maccess(p+j*4096);
    //}
    // uint64_t end = rdtsc();
    // pmu_count = (end-begin);
    // asm("VERW %AX\t\n");
    __asm volatile(
        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        "movl %%eax , %%esi\n\t"

        "mov (%%rbx), %%rax\n\t"

        "mfence\n\t"
        "rdpmc\n\t"
        "mfence\n\t"
        "sub %%esi, %%eax\n\t"

        : "=a"(pmu_count)       // outputs
        : "c"(pctr_arg), "b"(p) // inputs
    );
    // if (pmu_count != 0)
    //     printf("count:%ld\n", pmu_count);
    return pmu_count;
}

int main(void)
{

    char *mapping = (char *)mmap(0, 32 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);
    memset(mapping, 1, 4096);

    uint32_t count = 1000;
    uint64_t sum = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        sum += read_pmu(mapping);
    }
    printf("pmu_count:%ld\n", sum);

    return 0;
}
