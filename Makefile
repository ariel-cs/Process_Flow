CC = gcc
CFLAGS = -Wall -Wextra -g -std=c11
SRC = $(wildcard source/*.c)
OBJDIR = objects
OBJ = $(patsubst source/%.c, $(OBJDIR)/%.o, $(SRC))
TARGET = processflow

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

$(OBJ): | $(OBJDIR)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: source/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJDIR)/*.o

.PHONY: all clean
