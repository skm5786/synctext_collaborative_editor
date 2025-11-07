# Makefile for SyncText (Modularized with src directory)
#

# --- Compiler and Flags ---
CXX = g++
# Include the src directory for headers
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Isrc
LDFLAGS = -lrt -lpthread

# --- Target ---
TARGET = editor

# --- Directories ---
SRC_DIR = src
OBJ_DIR = obj

# --- Files ---
# Find all .cpp files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)

# Create a list of object file paths in the obj directory
# e.g., src/main.cpp -> obj/main.o
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SOURCES))

# Find all .h files for dependency tracking
HEADERS = $(wildcard $(SRC_DIR)/*.h)

# Default rule: build the target
all: $(TARGET)

# Rule to link the target executable
$(TARGET): $(OBJECTS)
	@echo "Linking $(TARGET)..."
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	@echo "Build complete: ./$(TARGET)"

# Rule to compile .cpp files into .o files
# This rule creates the obj directory if it doesn't exist (order-only dependency)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS) | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Rule to create the object directory
$(OBJ_DIR):
	@echo "Creating directory $(OBJ_DIR)..."
	@mkdir -p $(OBJ_DIR)

# Clean rule
clean:
	@echo "Cleaning up..."
	rm -f $(TARGET)
	rm -rf $(OBJ_DIR)

# Phony rule to prevent conflicts with a file named 'clean' or 'all'
.PHONY: all clean