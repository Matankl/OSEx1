# Makefile for compiling and profiling the max_subarray_sum programs

# Compiler
CXX = g++

# Compiler flags
CXXFLAGS = -pg -O2 -std=c++11

# Target executables
CUBIC_TARGET = max_subarray_sum_cubic
QUADRATIC_TARGET = max_subarray_sum_quadratic
LINEAR_TARGET = max_subarray_sum_linear

# Source files
CUBIC_SRC = main_cubic.cpp
QUADRATIC_SRC = main_quadratic.cpp
LINEAR_SRC = main_linear.cpp

# Object files
CUBIC_OBJ = $(CUBIC_SRC:.cpp=.o)
QUADRATIC_OBJ = $(QUADRATIC_SRC:.cpp=.o)
LINEAR_OBJ = $(LINEAR_SRC:.cpp=.o)

# Default target
all: $(CUBIC_TARGET) $(QUADRATIC_TARGET) $(LINEAR_TARGET)

# Rule to build the cubic target executable
$(CUBIC_TARGET): $(CUBIC_OBJ)
	$(CXX) $(CXXFLAGS) -o $(CUBIC_TARGET) $(CUBIC_OBJ)

# Rule to build the quadratic target executable
$(QUADRATIC_TARGET): $(QUADRATIC_OBJ)
	$(CXX) $(CXXFLAGS) -o $(QUADRATIC_TARGET) $(QUADRATIC_OBJ)

# Rule to build the linear target executable
$(LINEAR_TARGET): $(LINEAR_OBJ)
	$(CXX) $(CXXFLAGS) -o $(LINEAR_TARGET) $(LINEAR_OBJ)

# Rule to compile the source files into object files
%.o: %.cpp Part5.hpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to run the cubic program and generate the profiling data
run_cubic: $(CUBIC_TARGET)
	./$(CUBIC_TARGET)

# Rule to run the quadratic program and generate the profiling data
run_quadratic: $(QUADRATIC_TARGET)
	./$(QUADRATIC_TARGET)

# Rule to run the linear program and generate the profiling data
run_linear: $(LINEAR_TARGET)
	./$(LINEAR_TARGET)

# Rule to analyze the profiling data for cubic
profile_cubic: run_cubic
	gprof $(CUBIC_TARGET) gmon.out > analysis_cubic.txt
	cat analysis_cubic.txt

# Rule to analyze the profiling data for quadratic
profile_quadratic: run_quadratic
	gprof $(QUADRATIC_TARGET) gmon.out > analysis_quadratic.txt
	cat analysis_quadratic.txt

# Rule to analyze the profiling data for linear
profile_linear: run_linear
	gprof $(LINEAR_TARGET) gmon.out > analysis_linear.txt
	cat analysis_linear.txt

# Clean up generated files
clean:
	rm -f $(CUBIC_TARGET) $(QUADRATIC_TARGET) $(LINEAR_TARGET) $(CUBIC_OBJ) $(QUADRATIC_OBJ) $(LINEAR_OBJ) gmon.out analysis_*.txt

.PHONY: all run_cubic run_quadratic run_linear profile_cubic profile_quadratic profile_linear clean
