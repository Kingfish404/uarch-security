CORES=3,7
# CORES=0,4

all: 
	gcc -O3 -o loadport.out loadport.c

run:
	gcc -O3 -o loadport.out loadport.c
	taskset -c $(CORES) ./loadport.out

prepare:
	# huge pages manually allocate
	echo 16 | sudo tee /proc/sys/vm/nr_hugepages

clean:
	rm *.out
