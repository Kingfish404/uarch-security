all: main.c
	gcc main.c -Os -o leak.out

attack:
	gcc main.c -Os -o leak.out
	taskset -c 2 ./leak.out

clean:
	@rm -rf *.out
