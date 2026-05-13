# io_uring HTTP Server

- This is a single-threaded HTTP Server uses `io_uring` to handle all of its system calls involved in a request (`accept`, `recv`, `send`, `close`). 
- The intent of this HTTP Server is to serve static files from the public directory.
- This HTTP Server implements a subset of [HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616).

## Prerequisites

- `clang++` (I used 20.1.8)
- `make`
- `liburing`: `sudo apt install liburing-dev`

## Building

In the project root, run:

```
make 
```

## Running

```
cd build
./build/main
```

## Background

- This is my first time writing any C++ code.
- I took an Operating Systems course, and I was really interested in learning modern system calls, that's where I found `io_uring`.
- I started taking a Networking Course, and that's where I learned the basics of the simple HTTP server.
- Therefore, I spent my time combining my knowledge from both courses to create this `io_uring` HTTP Server.

## Performance Benchmarks

I used [wrk](https://github.com/wg/wrk) to measure the server's queries per second (QPS) throughput:

```
ulimit -n 100000 # depends on distribution
wrk -c10k -d10s http://localhost:8080/
```

| CPU             | OS              | Optimisation Flags | Results          |
| :-------------- | :-------------- | :----------------: | :--------------- |
| Intel i5-13600K | WSL - Debian 12 |         -          | ~157k - 159k QPS |
| Intel i5-13600K | WSL - Debian 12 |   `-O3 -DNDEBUG`   | ~178k - 189k QPS |
| Ryzen 7 4800H   | CachyOS         |         -          | ~78k - 80k QPS   |
| Ryzen 7 4800H   | CachyOS         |   `-O3 -DNDEBUG`   | ~89k - 92k QPS   |
| Raspberry Pi5   | Debian 12       |         -          | ~85k QPS         |
| Raspberry Pi5   | Debian 12       |   `-O3 -DNDEBUG`   | ~107k QPS        |
| Intel i3-8100   | Debian 13       |         -          | ~104k -107k QPS  |
| Intel i3-8100   | Debian 13       |   `-O3 -DNDEBUG`   | ~116 - 119k QPS  |

## Possible Future Items

- Support more methods
- Thread pools for requests
- Make it a library where users can declare their own endpoints and handlers

## References

- [HTTP/1.1](https://datatracker.ietf.org/doc/html/rfc2616)
- [io_uring](https://kernel.dk/io_uring.pdf)
- [liburing](https://github.com/axboe/liburing)
