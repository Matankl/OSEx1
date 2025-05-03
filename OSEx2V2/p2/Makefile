CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: p2

p2: mync.o
	$(CC) $(CFLAGS) $(LDFLAGS) mync.o -o p2

mync.o: mync.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f mync.o p2

