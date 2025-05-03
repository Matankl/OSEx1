# Compiler and Flags
CXX = g++ # CXX is the compiler
CXXFLAGS = -std=c++17 -Wall -pg #-std=c++17 is the flag for C++17 standard, -Wall is to show all warnings, -pg is for profiling
CCOVFLAGS = -g -fprofile-arcs -ftest-coverage -Og # -g is for debugging, -fprofile-arcs is for profiling, -ftest-coverage is for coverage, -Og is for optimization

# Files
SRC = mainRandom.cpp graph_and_Euler.cpp Graph.hpp
TARGET = mainRandom

# Default target: Compile the program
all: mainRandom

mainRandom: mainRandom.o graph_and_Euler.o
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -o $(TARGET) mainRandom.o graph_and_Euler.o

#init .o files
mainRandom.o: mainRandom.cpp graph_and_Euler.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -c mainRandom.cpp graph_and_Euler.cpp

graph_and_Euler.o: graph_and_Euler.cpp Graph.hpp
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -c graph_and_Euler.cpp

# Clean build files
clean:
	rm -f mainRandom  *.gcda *.gcno *.o gmon.out
