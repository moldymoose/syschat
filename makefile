CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS =
BUILD = build

SRC := $(wildcard src/*.c)
OBJ := $(patsubst src/%, $(BUILD)/%, $(SRC:.c=.o))

all: $(BUILD)/syschat 

$(BUILD)/syschat: $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
