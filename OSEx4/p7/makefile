#this is a makefile to compile hello.c with the name hello and valgraind
#

# Compiler and Flags
CXX = gcc # CXX is the compiler
CXXFLAGS = -Wall -pg #-Wall is to show all warnings, -pg is for profiling
CCOVFLAGS = -g -fprofile-arcs -ftest-coverage -Og # -g is for debugging, -fprofile-arcs is for profiling, -ftest-coverage is for coverage, -Og is for optimization

# Files
SRC = race.c
TARGET = race

# Default target: Compile the program
all: race

race: race.o
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -o $(TARGET) race.o

#init .o files
race.o: race.c
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -c race.c

# Clean build files
clean:
	rm -f race  *.gcda *.gcno *.o gmon.out