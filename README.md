# SyncText Project (Full Implementation)

This project is a lock-free, CRDT-based collaborative text editor. This is the final version, implementing Parts 1, 2, and 3. It supports multi-user discovery, change broadcasting, and conflict-free merging using a Last-Writer-Wins (LWW) strategy.

## Dependencies

* A C++17 compliant compiler (e.g., `g++`)
* `pthread` library (for threads and atomics)
* `rt` library (for shared memory and POSIX message queues)
* A **Linux-based OS** (e.g., Ubuntu 20.04) is required, as macOS does not support `mqueue.h`.

## Compilation Instructions

You can compile the project using the provided `Makefile`:

```bash
make
