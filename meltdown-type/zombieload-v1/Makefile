all: main.c
	gcc main.c -Os -Wall -o leak.out -std=gnu99

attack:
	sudo taskset -c 3 ./leak.out

clean:
	@rm -rf *.out
