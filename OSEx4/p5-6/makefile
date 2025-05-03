#this is a makefile to compile hello.c with the name hello and valgraind
#

# Compiler and Flags
CXX = gcc # CXX is the compiler
CXXFLAGS = -Wall -pg #-Wall is to show all warnings, -pg is for profiling
CCOVFLAGS = -g -fprofile-arcs -ftest-coverage -Og # -g is for debugging, -fprofile-arcs is for profiling, -ftest-coverage is for coverage, -Og is for optimization

# Files
SRC = hello.c
TARGET = hello

# Default target: Compile the program
all: hello

hello: hello.o
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -o $(TARGET) hello.o

#init .o files
hello.o: hello.c
	$(CXX) $(CXXFLAGS) $(CCOVFLAGS) -c hello.c

# Clean build files
clean:
	rm -f hello  *.gcda *.gcno *.o gmon.out