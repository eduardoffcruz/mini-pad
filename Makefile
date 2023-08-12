CC = gcc # compiler
CFLAGS = -std=c99 -Iinclude -Wall -Wextra -pedantic # compilation flags

# Define directory names
SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

# generate .o filenames based on .c filenames
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# Set executable filename
TARGET = minipad

# Default rule that is executed when 'make' is ran without arguments
all: $(TARGET)

# This rule specifies how to build the executable
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(OBJ_FILES) -o $(BUILD_DIR)/$(TARGET) -lm -lz

# Rule for compiling individual src files to obj files.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)  # The | ensures the directory is created before compiling
	$(CC) $(CFLAGS) -c $< -o $@

# Create the build and obj directories if they don't exist
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

debug: CFLAGS += -g
debug: all

# used to remove the obj files and the executable file when 'make clean' in ran
clean:
	rm -rf $(BUILD_DIR)
#	rm -f $(OBJ_FILES) $(BUILD_DIR)/$(TARGET)


#target: dependencies
#    commands
#
#Target: The target is the file (or output) that the rule will build. It can be an executable, an object file, or any other file that is part of the build process.
#Dependencies: Dependencies are the files that the target depends on. If any of the dependencies have changed since the last build, the target needs to be rebuilt. Dependencies are usually source code files, header files, or other intermediary files.
#Commands: The commands section contains the instructions for how to build the target. These are the shell commands that are executed when the rule is invoked. They typically include compilation commands, linking commands, or any other steps required to generate the target.