# Makefile to build server and client applications

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17

# Default target to build both server and client
all: server client

# Build the server executable from Server.cpp
server: Server.cpp
	$(CXX) $(CXXFLAGS) -o server Server.cpp

# Build the client executable from Client.cpp
client: Client.cpp
	$(CXX) $(CXXFLAGS) -o client Client.cpp

# Clean target to remove generated files
clean:
	rm -f server client
