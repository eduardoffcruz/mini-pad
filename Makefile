CC = gcc # compiler
CFLAGS = -Wall -Wextra -pedantic -std=c99 -Iinclude -g # compilation flags

# Define directory names
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# generate .o filenames based on .c filenames
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Set executable filename
TARGET = text_editor

# Default rule that is executed when 'make' is ran without arguments
all: $(TARGET)

# This rule specifies how to build the executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(BUILD_DIR)/$(TARGET)

# Rule for compiling individual src files to obj files.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)  # The | ensures the directory is created before compiling
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build and obj directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# used to remove the obj files and the executable file when 'make clean' in ran
clean:
	rm -rf $(BUILD_DIR)
# 	rm -rf $(OBJ_FILES) $(BUILD_DIR)/$(TARGET)

