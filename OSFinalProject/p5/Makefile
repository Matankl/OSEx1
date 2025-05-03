# Makefile to build two different MST servers and one client
# Assumes the three .cpp files exist in the same directory:
#   - server_pipeline.cpp
#   - server_leader_follower.cpp
#   - client.cpp

CXX = g++
CXXFLAGS = -Wall -std=c++17 -g -O0 -fprofile-arcs -ftest-coverage

# Default target: build everything
all: server_pipeline server_leader_follower client

# Build the pipeline/ActiveObject server
server_pipeline: server_pipeline.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the Leader-Follower server
server_leader_follower: server_leader_follower.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Build the client
client: client.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^

# Clean up build artifacts
clean:
	rm -f server_pipeline server_leader_follower client *.gcno *.gcda *.gcov
