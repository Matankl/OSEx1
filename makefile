# Makefile for compiling and profiling the max_subarray_sum program

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -pg -O2 -std=c++11

# Target executable
TARGET = max_subarray_sum

# Source files
SRCS = Part5.cpp

# Object files
OBJS = $(SRCS:.cpp=.o)

# Default target
all: $(TARGET)

# Rule to build the target executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

# Rule to compile the source files into object files
%.o: %.cpp Part5.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to run the program and generate the profiling data
run: $(TARGET)
	./$(TARGET)

# Rule to analyze the profiling data
profile: run
	gprof $(TARGET) gmon.out > analysis.txt
	cat analysis.txt

# Clean up generated files
clean:
	rm -f $(TARGET) $(OBJS) gmon.out analysis.txt

.PHONY: all run profile clean
