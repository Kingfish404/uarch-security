#include <sched.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

static void *nopthread(void *dummy)
{
    while (1)
    {
        asm volatile("nop");
    }
}

size_t virt_to_phys(size_t vaddr)
{
    int pagemap = open("/proc/self/pagemap", O_RDONLY);
    if (pagemap < 0)
    {
        return 0;
    }
    uint64_t virtual_addr = (uint64_t)vaddr;
    size_t value = 0;
    off_t offset = (virtual_addr / 4096) * sizeof(value);
    int got = pread(pagemap, &value, sizeof(value), offset);
    if (got != sizeof(value))
    {
        return 0;
    }
    close(pagemap);

    uint64_t page_frame_number = value & ((1ULL << 54) - 1);
    if (page_frame_number == 0)
        return 0;
    return (value << 12) | ((size_t)vaddr & 0xFFFULL);
}

const char *strings[] = {"MicroArchitecture"};

int main(int argc, char *argv[])
{
    static pthread_t *load_thread;
    load_thread = malloc(sizeof(pthread_t));
    int r_nop_thread = pthread_create(load_thread, 0, nopthread, 0);

    const char *secret = strings[0];
    int len = strlen(secret);

    printf("\x1b[32;1m[+]\x1b[0m Secret: \x1b[33;1m%s\x1b[0m\n", secret);

    size_t paddr = virt_to_phys((size_t)secret);
    if (!paddr)
    {
        printf("\x1b[31;1m[!]\x1b[0m Program requires root privileges (or read access to /proc/<pid>/pagemap)!\n");
        pthread_cancel(*load_thread);
        free(load_thread);
        exit(1);
    }

    printf("\x1b[32;1m[+]\x1b[0m Physical address of secret: \x1b[32;1m0x%zx\x1b[0m\n", paddr);
    // "echo %zx > PHYSICAL_ADDRESS_OF_SECRET", paddr
    char *cmd = malloc(100);
    sprintf(cmd, "echo 0x%zx > PHYSICAL_ADDRESS_OF_SECRET", paddr);
    system(cmd);
    printf("\x1b[32;1m[+]\x1b[0m Exit with \x1b[37;1mCtrl+C\x1b[0m if you are done reading the secret\n");
    while (1)
    { // keep string cached for better results
        volatile size_t dummy = 0, i;
        for (i = 0; i < len; i++)
        {
            dummy += secret[i];
        }
        sched_yield();
    }

    pthread_cancel(*load_thread);
    free(load_thread);

    return 0;
}
