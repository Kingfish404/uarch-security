all: x86

x86: main_x86
arm: main_arm

main_x86: main.c
	gcc main.c -o spectre.out -Os -I./../include

main_arm: main.c
	aarch64-linux-gnu-gcc -march=armv8-a -D__ARM_ARCH_8A__ -static -Os main.c -o spectre.out -I./../

clean:
	rm -f *.out
