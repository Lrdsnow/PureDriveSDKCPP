# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -std=c++17 -O2
LDFLAGS = -lbluetooth -ldbus-1 -pthread -lsimpleble

# Directories
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = build/obj
BIN_DIR = build/bin
LIB_DIR = build/lib

TARGET = $(BIN_DIR)/PureDrive
LIB_TARGET = $(LIB_DIR)/libPureDrive.a

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/anki_sdk/*.c)
OBJECTS = $(SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)

# Default target
all: $(TARGET)

# Library target
lib: $(LIB_TARGET)

# Compile and link executable
$(TARGET): $(OBJECTS)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile and create a static library, excluding main_debug.cpp
$(LIB_TARGET): $(OBJECTS)
	@mkdir -p $(LIB_DIR)
	$(AR) rcs $@ $(filter-out $(OBJ_DIR)/main_debug.o, $(OBJECTS))

# Compile source files into object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up the build
clean:
	rm -rf $(BUILD_DIR)

# Rebuild
rebuild: clean all

.PHONY: all clean rebuild lib
