CC = gcc
CFLAGS = -Wall -Iinclude

SRC = src
INCLUDE = include
BIN = bin

.PHONY: all
all: main

main: $(SRC)/$(wildcard *)
	$(CC) $(CFLAGS) -o $(BIN)/$@ $^

.PHONY: run_client
run_client: $(BIN)/main
	./$(BIN)/main

.PHONY: clean
clean:
	rm -rf $(BIN)/main
