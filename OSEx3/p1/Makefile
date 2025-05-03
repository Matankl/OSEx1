CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: p1

p1: Kosaraju.o
	$(CC) $(CFLAGS) $(LDFLAGS) Kosaraju.o -o p1

Kosaraju.o: Kosaraju.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f Kosaraju.o p1
