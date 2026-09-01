CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude
LDLIBS = -lncurses

SRC = $(wildcard src/*.c)
TARGET = monitor

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(LDLIBS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)