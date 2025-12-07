CC = gcc
CFLAGS = -Wall -Iinclude

SRC = src
INCLUDE = include
BIN = bin

# URL for debugging
#URL1 = ftp://ftp.up.pt/pub/gnu/emacs/elisp-manual-21-2.8.tar.gz
URL1 = ftp://ftp.up.pt/pub/debian/README.html
URL2 = ftp://demo:password@test.rebex.net/readme.txt
URL3 = ftp://anonymous:anonymous@ftp.bit.nl/speedtest/100mb.bin

.PHONY: all
all: main

main: $(SRC)/*.c
	$(CC) $(CFLAGS) -o $(BIN)/download $^

# simple command for easier debugging
.PHONY: run_client_debug
run_client_debug: main
	@echo ""
	@echo "Testing 1st case"
	./$(BIN)/download $(URL1)
	@echo ""
	@echo "Testing 2nd case"
	./$(BIN)/download $(URL2)
	@echo ""
	@echo "Testing 3rd case"
	./$(BIN)/download $(URL3)

.PHONY: clean
clean:
	rm -rf $(BIN)/download
