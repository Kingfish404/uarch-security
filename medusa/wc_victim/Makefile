all: secret.c
	gcc secret.c -Os -o secret.out

run:
	gcc secret.c -Os -o secret.out
	taskset -c 6 ./secret.out

clean:
	@rm -rf *.out