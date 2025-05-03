# Compiler and flags
CXX := g++
CXXFLAGS := -Wall -Wextra -std=c++17

# Name of the final executable
TARGET := MSTgraph

# Default rule: build the executable
all: $(TARGET)

# Rule to build the MSTgraph executable from MSTgraph.cpp
$(TARGET): MSTgraph.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

# Optional clean rule to remove generated files
clean:
	rm -f $(TARGET)
