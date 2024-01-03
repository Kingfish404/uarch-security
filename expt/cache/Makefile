all: flush_cache.c
	gcc flush_cache.c -O0 -o test.out -std=gnu99

l:
	gcc flush_l_cache.c -O0 -o test.out

evict:
	gcc evict_cache.c -O0 -o test.out

defence:
	gcc defence.c -O0 -o test.out

run:
	taskset -c 3 ./test.out

clean:
	@rm -rf *.out
