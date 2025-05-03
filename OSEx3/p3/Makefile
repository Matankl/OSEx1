CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: p3

p3: main.o
	$(CC) $(CFLAGS) $(LDFLAGS) main.o -o p3

main.o: main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.o p3
