#include <stdio.h>
#include <memory.h>

void maccess(void *p)
{
  asm volatile("movq (%0), %%rax\n"
               :
               : "c"(p)
               : "rax");
}

char secret_str[] = "helloworld";

char __attribute__((aligned(4096))) secret[8192];
char str[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJK";

int main(int argc, char *argv[])
{
  char c;

  printf("Loading secret map: [A-Za-z]\n");
  for (unsigned int i = 0; i < sizeof(secret_str); i++)
  {
    str[i] = secret_str[i];
  }

  for (unsigned int i = 0; i < 4096 / 64; i++)
  {
    for (unsigned int j = 0; j < 64; j++)
    {
      if (j < sizeof(str))
      {
        c = str[j];
        memset(secret + i * 64 + j, c, 1);
      }
      else
      {
        memset(secret + i * 64 + j, 0, 1);
      }
    }
  }
  for (unsigned int i = 4096 / 64; i < 8192 / 64; i++)
  {
    for (unsigned int j = 0; j < 64; j++)
    {
      if (j < sizeof(str))
      {
        c = str[j];
        memset(secret + i * 64 + j, c, 1);
      }
      else
      {
        memset(secret + i * 64 + j, 0, 1);
      }
    }
  }

  // load value all the time
  while (1)
  {
    for (int i = 0; i < 8185; i++)
    {
      maccess(secret + i);
    }
  }
}
