#include <stdint.h>
#include <memory.h>

const size_t CACHE_MISS = 150;

namespace x86_64
{
    class ASMUtils
    {
    public:
        static void nop()
        {
            __asm__ volatile("nop");
        }

        static uint64_t rdtsc()
        {
            uint64_t a, d;
            __asm__ volatile("mfence");
            __asm__ volatile("rdtsc"
                             : "=a"(a), "=d"(d));
            a = (d << 32) | a;
            __asm__ volatile("mfence");
            return a;
        }

        static void flush(void *p) { __asm__ volatile("clflush 0(%0)\n"
                                                      :
                                                      : "c"(p)
                                                      : "rax"); }

        static void maccess(void *p) { __asm__ volatile("movq (%0), %%rax\n"
                                                        :
                                                        : "c"(p)
                                                        : "rax"); }

        static void mfence() { __asm__ volatile("mfence"); }

        static int flush_reload(void *ptr)
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