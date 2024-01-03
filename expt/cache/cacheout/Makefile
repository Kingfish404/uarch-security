all: main.c
	gcc main.c -Os -Wall -o leak.out -std=gnu99

attack:
	gcc main.c -Os -Wall -o leak.out -std=gnu99
	sudo taskset -c 7 ./leak.out

attack_tsx:
	gcc main_tsx.c -Os -Wall -o leak.out -std=gnu99
	sudo taskset -c 7 ./leak.out

clean:
	@rm -rf *.out
