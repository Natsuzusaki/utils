# MIT License
#
# Copyright (c) 2025 Ragib Asif
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
#################################################################################
#
# Make commands:
# `make`
# `make all`
# `make clean`

# Object files are compiled with the appropriate flags for each target.
#  Uses `-g` for debugging symbols, `-O2` for optimization, and `-DDEBUG` to enable debug-specific code. Enables all warnings (`-Wall -Wextra -pedantic`) and treats warnings as errors (`-Werror`).
# `-O3` for maximum optimization and `-DNDEBUG` to disable assertions.
# `all`: building the binary
# `clean`: Removes obj and bin files
#
# use tabs instead of spaces
#

# Compiler
CC = gcc

# compiler flags
# which flags to use example: -Wall -Werror -Wextra -02 -pedantic -DDEBUG for debug build
# -03 -DNDEBUG for release build
CFLAGS = -Wall -Wextra -O2 -g -Werror -pedantic -DDEBUG

LDFLAGS = # which libraries to use: -lm -lefence

# Executable names
# name of the final program
TARGET = utils

# Source files
SRC = utils.c main.c

# Object files
# which object files are part of the final program
OBJ = utils.o main.o

all: $(TARGET)

# Link object files to create the executable.
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ) $(LDFLAGS)

# Compile source files into object files.
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files.
clean:
	rm -f $(OBJ) $(TARGET)

# Phony targets
.PHONY: all clean

