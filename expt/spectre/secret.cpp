#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>

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

char secret[] = "AD ASTRA ABYSSOSQUE";

int main()
{
    size_t paddr = libkdump_virt_to_phys((size_t)&secret);
    printf("secret is at %p, phy mem: %p, len: %lu\n", (size_t)secret, paddr, sizeof(secret));
    for (int i = 0; i <= sizeof(secret); i++)
    {
        char c = secret[i];
        if (i == sizeof(secret))
        {
            i = 0;
        }
    }

    return 0;
}