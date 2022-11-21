KVERS = $(shell uname -r)

# Kernel modules
obj-m += kernel_write.o

build: kernel_modules

kernel_modules:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) modules

clean:
	make -C /lib/modules/$(KVERS)/build M=$(CURDIR) clean

cache_time:
	rm -f cache_time && gcc -O0 -o cache_time cache_time.c && ./cache_time

signal:
	rm -f signal && gcc signal_jmp.cpp -march=native -o signal && ./signal
