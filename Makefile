RAYLIB_PATH = ../raylib/build/raylib/include
CC = clang
CFLAGS = -O0
LDFLAGS = -lraylib
SRC_DIR = src
BUILD_DIR = build

# List all source files here
SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Main target
main: $(OBJS)
	$(CC) $(CFLAGS) -o $(BUILD_DIR)/$@ $^ $(LDFLAGS)

# Rule to compile source files into object files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c -o $@ $< $(LDFLAGS)

$(BUILD_DIR):
	mkdir -p $@

.PHONY: run clean

run: main
	./$(BUILD_DIR)/main

clean:
	rm -rf $(BUILD_DIR)
