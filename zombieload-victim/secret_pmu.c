#include <stdio.h>
#include <memory.h>
#include <stdint.h>
#include <sys/mman.h>
#include <linux/mman.h>
#include "../../pmu-linux-embedded/include/pmu.h"

void maccess(void *p)
{
  asm volatile("movq (%0), %%rax\n"
               :
               : "c"(p)
               : "rax");
}

char __attribute__((aligned(4096))) secret[8192];

int main(int argc, char *argv[])
{
  char key = 'X';
  char *mem_evict = (char *)mmap(0, 320 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

  if (argc >= 2)
  {
    key = argv[1][0];
  }

  printf("Loading secret value '%c'...\n", key);

  memset(secret, key, 4096 * 2);
  uint64_t lo, hi, pmu_start = 0, pmu_end = 0, sum = 0, counter = 0;

  // load value all the time
  while (1)
  {
    for (int i = 0; i < 100; i++)
    {
      maccess(secret + i * 64);
      // for (uint64_t j = 0; j < 11; j += 1)
      // {
      //   maccess(&mem_evict[j * 4096 + i * 64]);
      // }
      READ_PMU_N(0x0, pmu_start, lo, hi);
      maccess(secret + i * 64);
      READ_PMU_N(0x0, pmu_end, lo, hi);
      sum += (pmu_end - pmu_start);
    }
    counter++;
    printf("%ld\n", sum);
    sum = 0;
  }
}
