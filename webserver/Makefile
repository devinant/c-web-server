TARGET = httpd
CC = gcc
OBJ_DIR = objects
SRC_DIR = src
INC_DIR = include

CFLAGS = -g -Wall -I$(INC_DIR) -c
LFLAGS = -o

SOURCES = $(wildcard $(SRC_DIR)/*.c)
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

.PHONY: make_object_dir

all: make_object_dir $(TARGET)

make_object_dir:
	@mkdir -p $(OBJ_DIR)

$(TARGET): $(OBJECTS)
	$(CC) $(LFLAGS) $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	@rm $(TARGET) 
	@rm -rf $(OBJ_DIR)
