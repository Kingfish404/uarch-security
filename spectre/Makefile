CFLAGS = -std=c++11 -Wall -O0

PROGRAM = spectre.out
SOURCE  = main.cpp utils.hpp
     
all: $(PROGRAM)
     
$(PROGRAM): $(SOURCE) ; $(CC) $(CFLAGS) -o $(PROGRAM) $(SOURCE)
     
clean: ; rm -f $(PROGRAM)
