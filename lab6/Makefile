CC = gcc
CFLAGS = -Wall -Wextra -O2

PIPE = pipe_example
FIFO_WRITER = fifo_writer
FIFO_READER = fifo_reader

all: $(PIPE) $(FIFO_WRITER) $(FIFO_READER)

$(PIPE): pipe_example.c
	$(CC) $(CFLAGS) -o $(PIPE) pipe_example.c

$(FIFO_WRITER): fifo_writer.c
	$(CC) $(CFLAGS) -o $(FIFO_WRITER) fifo_writer.c

$(FIFO_READER): fifo_reader.c
	$(CC) $(CFLAGS) -o $(FIFO_READER) fifo_reader.c

run_pipe: $(PIPE)
	@echo "=== Запуск pipe-примера ==="
	./$(PIPE)

run_fifo: $(FIFO_READER) $(FIFO_WRITER)
	@echo "=== Запуск FIFO-примера ==="
	@echo "Откройте два терминала:"
	@echo "1) В первом: ./$(FIFO_READER)"
	@echo "2) Во втором: ./$(FIFO_WRITER)"
	@echo "-------------------------------------------"

clean:
	rm -f $(PIPE) $(FIFO_WRITER) $(FIFO_READER)
	rm -f /tmp/myfifo
	@echo "Все очищено."

.PHONY: all clean run_pipe run_fifo
