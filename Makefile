CC = gcc
CFLAGS = -Wall -fsanitize=address -Wextra -g

TARGET = main
OBJ = malloc_usage.o rgs_alloc.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean