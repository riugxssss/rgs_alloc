CC = gcc

CFLAGS = -Wall -Wextra -fsanitize=address -O1 -g -Iinclude

TARGET = main

SRC = src/rgs_alloc.c \
      src/rgs_freelist.c \
      src/rgs_functionality.c \
      test/malloc_usage.c

OBJ = $(SRC:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean