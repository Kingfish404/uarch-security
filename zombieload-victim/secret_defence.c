#include <stdio.h>
#include <memory.h>

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

  if (argc >= 2)
  {
    key = argv[1][0];
  }

  printf("Loading secret value '%c'...\n", key);

  memset(secret, key, 4096 * 2);

  // load value all the time
  while (1)
  {
    for (int i = 0; i < 100; i++)
    {
      // no use
      asm volatile("mfence\n\t");
      asm volatile("verw %ax\n\t");
      maccess(secret + i * 64);
      asm volatile("mfence\n\t");
      asm volatile("verw %ax\n\t");
    }
  }
}
