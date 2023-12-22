# Makefile

# Compiler and flags
CC = gcc
CFLAGS = -Wall

SRC = src/
INCLUDE = include/
BIN = bin/

# Object files
OBJ = $(BIN)/download.o $(BIN)/connection.o $(BIN)/utils.o

# Build executable
app: $(OBJ)
	$(CC) $(CFLAGS) -o app $(OBJ)

# Compile source files
$(BIN)/download.o: $(SRC)/download.c $(INCLUDE)/download.h
	$(CC) $(CFLAGS) -c $(SRC)/download.c -o $(BIN)/download.o

$(BIN)/connection.o: $(SRC)/connection.c $(INCLUDE)/connection.h
	$(CC) $(CFLAGS) -c $(SRC)/connection.c -o $(BIN)/connection.o

$(BIN)/utils.o: $(SRC)/utils.c $(INCLUDE)/utils.h
	$(CC) $(CFLAGS) -c $(SRC)/utils.c -o $(BIN)/utils.o

# Clean command
clean:
	rm -f $(BIN)/*.o app
