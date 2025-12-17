CC = gcc
CFLAGS = -Wall -Wextra -std=c11

TARGET1 = task9_1
TARGET2 = sender
TARGET3 = receiver

all: $(TARGET1) $(TARGET2) $(TARGET3)

$(TARGET1): task9_1.c
	$(CC) $(CFLAGS) task9_1.c -o $(TARGET1) -pthread

$(TARGET2): sender.c
	$(CC) $(CFLAGS) sender.c -o $(TARGET2)

$(TARGET3): receiver.c
	$(CC) $(CFLAGS) receiver.c -o $(TARGET3)

clean:
	rm -f $(TARGET1) $(TARGET2) $(TARGET3)
