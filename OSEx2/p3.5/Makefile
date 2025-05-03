CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: p3.5

p3.5: mync.o
	$(CC) $(CFLAGS) $(LDFLAGS) mync.o -o p3.5

mync.o: mync.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f mync.o p3.5

