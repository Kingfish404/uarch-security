all:
	make spectre-v1 -s
	make spectre-v2 -s
	make spectre-v4 -s
	make spectre-v5 -s

spectre-v1:
	cd spectre-v1-PHT && make && ./spectre.out

spectre-v2:
	cd spectre-v2-BTB && make && ./spectre.out

spectre-v4:
	cd spectre-v4-STL && make && ./spectre.out

spectre-v5:
	cd spectre-v5-RSB && make && taskset -c 1 ./spectre.out

clean:
	cd spectre-v1-PHT && make clean
	cd spectre-v2-BTB && make clean
	cd spectre-v4-STL && make clean
	cd spectre-v5-RSB && make clean
