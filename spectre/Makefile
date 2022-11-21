CFLAGS = -std=c++11 -Wall -O0
SOURCE-GENERAL = utils.hpp

MAIN = spectre
SOURCE  = main.cpp $(SOURCE-GENERAL)

MAIN-SIMPLE = spectre_simple
SOURCE-SIMPLE  = main_simple.cpp $(SOURCE-GENERAL)

all: $(MAIN) $(MAIN-SIMPLE)

$(MAIN): $(SOURCE) ; $(CC) $(CFLAGS) -o $(MAIN) $(SOURCE)

$(MAIN-SIMPLE): $(SOURCE-SIMPLE) ; $(CC) $(CFLAGS) -o $(MAIN-SIMPLE) $(SOURCE-SIMPLE)

clean: ; rm -f $(MAIN) $(MAIN-SIMPLE)

run:
	make clean && make && ./spectre

run-simple:
	make clean && make && ./spectre_simple

