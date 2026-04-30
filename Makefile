CC = clang++-20
CFLAGS = -Wall

compile: build/server

build/server: server.cpp | build
	$(CC) $(CFLAGS) server.cpp -o build/server -luring

build:
	mkdir -p build
