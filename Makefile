CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Iinclude

SRC = $(wildcard src/*.c)
TARGET = monitor

all:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

run: all