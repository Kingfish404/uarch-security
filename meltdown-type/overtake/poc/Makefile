CC :=gcc
CPP :=g++
CFLAGS :=-static -pthread -Wno-attributes -m64
CPPFLAGS :=
C_SOURCES :=$(wildcard *.c)
C_EXECUTABLE :=$(C_SOURCES:.c=.c.out)
CPP_SOURCES :=$(wildcard *.cpp)
CPP_EXECUTABLE :=$(CPP_SOURCES:.cpp=.cpp.out)
 
all:$(C_EXECUTABLE) $(CPP_EXECUTABLE)

%.c.out: %.c
	$(CC) $(CFLAGS) $< -o $@

%.cpp.out: %.cpp
	$(CPP) $(CPPFLAGS) $< -o $@

clean:
	rm -rf $(C_EXECUTABLE)
	rm -rf $(CPP_EXECUTABLE)
	rm -rf PHYSICAL_ADDRESS_OF_SECRET
