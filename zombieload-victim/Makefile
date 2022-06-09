all: secret.c
	gcc secret.c -O3 -o secret.out -std=gnu99

str:
	gcc secret_str.c -O3 -o secret.out -std=gnu99

defence:
	gcc secret_defence.c -O3 -o secret.out -std=gnu99

pmu:
	gcc secret_pmu.c -O3 -o secret.out -std=gnu99

run:
	taskset -c 3 ./secret.out

clean:
	@rm -rf *.out
