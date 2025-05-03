CC = gcc
CFLAGS = -Wall -g
LIBS = -ldl -lm -lstdc++
CCOV = -fprofile-arcs -ftest-coverage

all: progP4

progP4: Dijkstra.o
	$(CC) $(CFLAGS) $(CCOV) -o $@ $^ $(LIBS)

Dijkstra.o: Dijkstra.cpp
	$(CC) $(CFLAGS) $(CCOV) -c -o $@ $<

clean:
	rm -f *.o progP4

