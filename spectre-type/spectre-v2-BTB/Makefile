LIB_PATH :=./../include
CFLAGS += -O0 -I$(LIB_PATH)

PROGRAM = spectre.out
SOURCE  = spectrev2.c
LIBC = $(wildcard $(LIB_PATH)/*/*.c)

all: $(PROGRAM)

$(PROGRAM): $(SOURCE) ; $(CC) $(CFLAGS) -o $(PROGRAM) $(SOURCE) $(LIBC)

clean: ; rm -f $(PROGRAM)
