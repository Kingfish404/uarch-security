LIB_PATH :=./../include

all: x86

x86: main_x86
arm: main_arm

main_x86: main.c
	gcc main.c -o spectre.out -Os -I$(LIB_PATH) -lpthread --std=gnu99

main_arm: main.c
	aarch64-linux-gnu-gcc -march=armv8-a -D__ARM_ARCH_8A__ -static -Os main.c -o spectre.out -I./../ -lpthread

clean:
	rm -f *.out

main_gadget:
	gcc main-gadget.c $(LIBC) -o spectre_gadget.out -Os -I$(LIB_PATH) -lpthread --std=gnu99