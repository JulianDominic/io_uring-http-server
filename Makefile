CC = clang++-20
CFLAGS = -Wall

compile: build/main

build/main: src/main.cpp src/server.cpp src/request.cpp src/utils.hpp | build
	$(CC) $(CFLAGS) src/main.cpp -o build/main -luring

build:
	mkdir -p build
