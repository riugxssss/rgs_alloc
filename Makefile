CC = gcc
CFLAGS = -Wall -fsanitize=address -O1 -Wextra -g 

TARGET = main
OBJ = malloc_usage.o rgs_alloc.o rgs_freelist.o rgs_functionality.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f $(OBJ) $(TARGET)

.PHONY: all clean