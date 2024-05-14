CC = gcc
CFLAGS = -Wall -g
LDFLAGS = -shared
LIBS = -ldl -lm -lstdc++

all: prog

libpoisson.so: PoissonMethod.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

prog: Main.o libpoisson.so
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

PoissonMethod.o : PoissonMethod.cpp PoissonMethod.hpp
	$(CC) $(CFLAGS) -c -fPIC -o $@ $< 

Main.o : Main.cpp PoissonMethod.hpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f *.o *.so prog
