#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

char secret[] = "AD ASTRA ABYSSOSQUE";

static sigjmp_buf jbuf;
static void catch_segv(int signal)
{
    // Roll back to the checkpoint set by sigsetjmp().
    siglongjmp(jbuf, 1);
}

int main()
{
    // The address of our secret data
    unsigned long kernel_data_addr = (unsigned long)&secret;
    kernel_data_addr = 0xffffffffc0544000;
    // Register a signal handler
    signal(SIGSEGV, catch_segv);
    int mark = sigsetjmp(jbuf, 1);
    printf("mark: %d\n", mark);
    if (mark == 0)
    {
        // A SIGSEGV signal will be raised.
        char kernel_data = *(char *)kernel_data_addr;
        // The following statement will not be executed.
        printf("Kernel data at address %lu is: %c\n",
               kernel_data_addr, kernel_data);
    }
    else
    {
        printf("Memory access violation!\n");
    }
    printf("Program continues to execute.\n");
    return 0;
}