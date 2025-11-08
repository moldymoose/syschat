CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude
LDFLAGS =
BUILD = build

SRC_CLIENT = src/client
SRC_SERVER = src/server
SRC_COMMON = src/common

SRC_CLIENT_FILES := $(wildcard $(SRC_CLIENT)/*.c)
SRC_SERVER_FILES := $(wildcard $(SRC_SERVER)/*.c)
SRC_COMMON_FILES := $(wildcard $(SRC_COMMON)/*.c)

OBJ_CLIENT := $(patsubst src/%, $(BUILD)/%, $(SRC_CLIENT_FILES:.c=.o)) $(patsubst src/%, $(BUILD)/%, $(SRC_COMMON_FILES:.c=.o))
OBJ_SERVER := $(patsubst src/%, $(BUILD)/%, $(SRC_SERVER_FILES:.c=.o)) $(patsubst src/%, $(BUILD)/%, $(SRC_COMMON_FILES:.c=.o))

all: $(BUILD)/client/syschat $(BUILD)/server/syschat-server

$(BUILD)/client/syschat: $(OBJ_CLIENT)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD)/server/syschat-server: $(OBJ_SERVER)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

$(BUILD)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)

.PHONY: all clean
