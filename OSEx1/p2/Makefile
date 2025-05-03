CC = g++
CFLAGS = -g3 -Wall -lm


all: Poisson

Poisson: Poisson.o 
	$(CC) $(CFLAGS) -o $@ $^ 


Poisson.o: Poisson.cpp
	$(CC) $(CFLAGS) -c -o $@ $<


clean:
	rm -f *.o *.a Poisson
