#include <stdint.h>

#define LOOP_CYCLE 1000

uint64_t rdtsc()
{
    uint64_t a, d;
    asm volatile("rdtsc"
                 : "=a"(a), "=d"(d));
    a = (d << 32) | a;
    return a;
}

uint64_t time1, time2;
void sync()
{
    time1 = rdtsc() / LOOP_CYCLE;
    do
    {
        time2 = rdtsc() / LOOP_CYCLE;
    } while (time2 == time1);
    time1 = time2;
}

int main(void)
{
a:
    sync();
    asm volatile("verw %ax\n\t");
    asm volatile("mfence\n\t");
    for (int i = 0; i < 10000; i++)
    {
        (i);
    }
    goto a;

    return 0;
}
