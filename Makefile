CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = $(wildcard source/*.c)
OBJ = $(SRC:.c=.o)
TARGET = processflow

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) source/*.o

.PHONY: all clean
