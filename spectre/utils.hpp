#ifndef UTILS_HPP
#define UTILS_HPP
#include <stdint.h>
#include <memory.h>

const size_t CACHE_MISS = 150;

namespace math
{
    bool is_prime(int a)
    {
        if (a < 2)
            return 0;
        for (int i = 2; i * i <= a; ++i)
            if (a % i == 0)
                return false;
        return true;
    }
}

namespace x86_64
{
    namespace asm_utils
    {
        void nop()
        {
            __asm__ volatile("nop");
        }

        uint64_t rdtsc()
        {
            uint64_t a, d;
            __asm__ volatile("mfence");
            __asm__ volatile("rdtsc"
                             : "=a"(a), "=d"(d));
            a = (d << 32) | a;
            __asm__ volatile("mfence");
            return a;
        }

        void flush(void *p) { __asm__ volatile("clflush 0(%0)\n"
                                               :
                                               : "c"(p)
                                               : "rax"); }

        void maccess(void *p) { __asm__ volatile("movq (%0), %%rax\n"
                                                 :
                                                 : "c"(p)
                                                 : "rax"); }

        void mfence() { __asm__ volatile("mfence"); }

        int flush_reload(void *ptr)
        {
            uint64_t start = 0, end = 0;

            start = rdtsc();
            maccess(ptr);
            end = rdtsc();

            mfence();

            flush(ptr);

            if (end - start < CACHE_MISS)
            {
                return 1;
            }
            return 0;
        }
    };

}
#endif