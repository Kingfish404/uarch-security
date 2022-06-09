all: main.c
	gcc main.c -Os -o leak.out -std=gnu99

str:
	gcc main_str.c -Os -o leak.out -std=gnu99

pmu:
	gcc main_pmu.c -Os -o leak.out -std=gnu99

leak.asm: main.c
	gcc main.c -Os -S -o leak.asm -std=gnu99

attack:
	taskset -c 7 ./leak.out

clean:
	@rm -rf leak
