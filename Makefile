CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = source/main.c
TARGET = processflow

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
