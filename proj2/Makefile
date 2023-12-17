CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

# List all your source files here
SOURCES = $(SRC)/download.c $(SRC)/connection.c $(SRC)/utils.c

.PHONY: downloader
downloader: $(SOURCES)
	$(CC) $(CFLAGS) -o download $^

.PHONY: clean
clean:
	rm -rf download
