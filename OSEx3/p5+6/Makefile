CC = g++
CFLAGS = 
LDFLAGS = -lstdc++

all: Server Client

Server: Server.o
	$(CC) $(CFLAGS) $(LDFLAGS) Server.o -o Server

Server.o: Server.cpp
	$(CC) $(CFLAGS) -c $< -o $@

Client: Client.o
	$(CC) $(CFLAGS) $(LDFLAGS) Client.o -o Client

Client.o: Client.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f main.o Server Client
