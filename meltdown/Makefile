CC=g++
CFLAGS = -Wall -O0

MAIN = meltdown
SOURCE  = main.cpp

MAIN-SIMPLE = meltdown_simple
SOURCE-SIMPLE  = main_simple.cpp

all: $(MAIN) $(MAIN-SIMPLE)

$(MAIN): $(SOURCE) ; $(CC) $(CFLAGS) -o $(MAIN) $(SOURCE)
$(MAIN-SIMPLE): $(SOURCE-SIMPLE) ; $(CC) $(CFLAGS) -o $(MAIN-SIMPLE) $(SOURCE-SIMPLE)

clean: ; rm -f $(MAIN) $(MAIN-SIMPLE)

run:
	make clean && make && ./meltdown

run-simple:
	make clean && make && ./meltdown_simple
