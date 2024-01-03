#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <sys/mman.h>
#include <unistd.h>
#include "../PTEditor/ptedit_header.h"
#include "../cacheutils.h"

#define VICTIM_REPSTOS 0

#define PAGE_SIZE 4096

unsigned char __attribute__((aligned(PAGE_SIZE))) src[PAGE_SIZE];
unsigned char __attribute__((aligned(PAGE_SIZE))) dst[PAGE_SIZE];

int main(int argc, char *argv[])
{
    memset(src, 0, PAGE_SIZE);
    memset(dst, 0, PAGE_SIZE);

    char *str =
        "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"
        "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ"
        "abcdefghijklmnopqrstuvwxyz123456"
        "abcdefghijklmnopqrstuvwxyz123456"
        "abcdefghijklmnopqrstuvwxyz123456"
        "abcdefghijklmnopqrstuvwxyz123456"
        "abcdefghijklmnopqrstuvwxyz123456"
        "abcdefghijklmnopqrstuvwxyz123456";

    memcpy(src, str, strlen(str));

    asm volatile(
        "mov %2, %%r11\n\t"
        "l:\n\t"
        "mov %%r11, %%rcx\n\t"
        "mov %0, %%rsi\n\t"
        "mov %1, %%rdi\n\t"
        "rep movsb\n\t"
        "jmp l\n\t"
        :
        : "r"(src), "r"(dst), "r"(strlen(str))
        : "r11");

#if VICTIM_REPSTOS
    asm volatile(
        "mov %1, %%r11\n\t"
        "l:\n\t"
        "mov %%r11, %%rcx\n\t"
        "mov %0, %%rdi\n\t"
        "mov $0x42, %%al\n\t"
        "rep stosb\n\t"
        "jmp l\n\t"
        :
        : "d"(src), "r"(strlen(str))
        : "r11");
#endif

    return 0;
}
