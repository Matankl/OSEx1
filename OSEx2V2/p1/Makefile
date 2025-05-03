CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: ttt

ttt: MainTTT.o
	$(CC) $(CFLAGS) $(LDFLAGS) MainTTT.o -o ttt

MainTTT.o: MainTTT.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f MainTTT.o ttt

