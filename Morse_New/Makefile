# Makefile to compile Morse Code.
# 
# Program name for executable
TARGET_NAME = MCode

# Project Home Directoy
PROJECT_HOME_DIR ?= .

# Build Directoy
BUILD_DIR ?= $(PROJECT_HOME_DIR)/Build

TARGET = $(BUILD_DIR)/$(TARGET_NAME)

CC = arm-linux-gnueabihf-gcc-7

# Source files
SOURCE = $(wildcard *.c)

# Object files
OBJS = $(SOURCE:%=$(BUILD_DIR)/%.o)

# Dependencies
DEP := $(OBJS:.o=.d) 

# Compiler and Linker Flags
CFLAGS += -Wall -c -ggdb
LFLAGS += -Wall -ggdb

# Links all the object files
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LFLAGS) -o $(TARGET)

# Compiles
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	@echo "Removing $(BUILD_DIR)";
	@$(RM) -r $(BUILD_DIR);


# include all Dependencies
-include $(DEP)

MKDIR_P ?= mkdir -p
RM ?= rm