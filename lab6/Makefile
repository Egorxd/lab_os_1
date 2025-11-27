CC = gcc
CFLAGS = -Wall

all: pipe_example fifo_writer fifo_reader

pipe_example: pipe_example.c
	$(CC) $(CFLAGS) -o $@ $<

fifo_writer: fifo_writer.c
	$(CC) $(CFLAGS) -o $@ $<

fifo_reader: fifo_reader.c
	$(CC) $(CFLAGS) -o $@ $<

run_pipe: pipe_example
	./pipe_example

run_fifo: fifo_reader fifo_writer
	./fifo_reader &
	sleep 1
	./fifo_writer

clean:
	rm -f pipe_example fifo_writer fifo_reader /tmp/myfifo

.PHONY: all run_pipe run_fifo clean
