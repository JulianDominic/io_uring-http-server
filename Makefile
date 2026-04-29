CC = clang++-20
CFLAGS = -Wall

compile: build/server build/client

build/server: server.cpp | build
	$(CC) $(CFLAGS) server.cpp -o build/server -luring

build/client: client.cpp | build
	$(CC) $(CFLAGS) client.cpp -o build/client

build:
	mkdir -p build
