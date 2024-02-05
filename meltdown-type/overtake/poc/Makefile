CC :=gcc
CPP :=g++
CFLAGS :=-static -pthread -Wno-attributes -m64
CPPFLAGS :=
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
all:$(C_EXECUTABLE) $(CPP_EXECUTABLE) zbl.out

%.c.out: %.c
	$(CC) $(CFLAGS) $< -o $@

%.cpp.out: %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(C_EXECUTABLE) zbl.out
	rm -rf $(CPP_EXECUTABLE)
	rm -rf PHYSICAL_ADDRESS_OF_SECRET

zbl.out: overtake_zbl_cmpsb_cache_cc.c
	$(CC) -Os -o $@ $<