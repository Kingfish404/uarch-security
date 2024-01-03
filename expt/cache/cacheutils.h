#include <inttypes.h>

void flush(void *p)
{
    asm volatile("clflush 0(%0)\n\t"
                 :
                 : "c"(p)
                 : "rax");
}

void maccess(void *p)
{
    asm volatile("movq (%0),%%rax\n\t"
                 :
                 : "c"(p)
                 : "rax");
}

void mfence() { asm volatile("mfence"); }

uint64_t rdtsc()
{
    uint64_t a, d;
    asm volatile("mfence");
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    asm volatile("mfence");
    return a;
}
