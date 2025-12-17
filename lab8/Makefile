CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = threads_mutex
SRC = threads_mutex.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all clean
