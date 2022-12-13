CFLAGS = -std=c++11 -Wall -O0
SOURCE-GENERAL = utils.hpp

MAIN = spectre
SOURCE  = main.cpp $(SOURCE-GENERAL)

MAIN-SEARCH = spectre_search
SOURCE-SEARCH  = main_search.cpp $(SOURCE-GENERAL)

MAIN-SIMPLE = spectre_simple
SOURCE-SIMPLE  = main_simple.cpp $(SOURCE-GENERAL)

all: $(MAIN) $(MAIN-SIMPLE) $(MAIN-SEARCH)

$(MAIN): $(SOURCE) ; $(CC) $(CFLAGS) -o $(MAIN) $(SOURCE)

$(MAIN-SEARCH): $(MAIN-SEARCH) ; $(CC) $(CFLAGS) -o $(MAIN-SEARCH) $(SOURCE-SEARCH)

$(MAIN-SIMPLE): $(MAIN-SIMPLE) ; $(CC) $(CFLAGS) -o $(MAIN-SIMPLE) $(SOURCE-SIMPLE)

clean: ; rm -f $(MAIN) $(MAIN-SIMPLE) $(MAIN-SEARCH)

run:
	make clean && make && ./spectre

run-simple:
	make clean && make && ./spectre_simple

