# Thread Pool

This project is a C++ workspace for exploring thread-pool design through hands-on practice, with implementations that are clean, structured, and useful for both learning and practical work.

It includes:

- a simple single-queue thread pool
- an advanced multi-queue thread pool

The code is easy to study and learn from, while also being structured in a way that feels practical, maintainable, and suitable for real-world use.

## What this project demonstrates

This repository shows how to:

- create and manage worker threads
- submit work items to a task queue
- coordinate execution safely with standard C++ primitives
- build a small but realistic concurrency abstraction
- organize a CMake-based project with separate libraries and tests

## Project structure

- [main.cpp](main.cpp) - example program that creates and uses the advanced thread pool
- [simple_thread_pool](simple_thread_pool) - simple thread-pool implementation
- [simple_thread_pool/include/simple_thread_pool.h](simple_thread_pool/include/simple_thread_pool.h) - public header for the simple pool
- [simple_thread_pool/src/simple_thread_pool.cpp](simple_thread_pool/src/simple_thread_pool.cpp) - implementation of the simple pool
- [simple_thread_pool/tests/simple_thread_pool_tests.cpp](simple_thread_pool/tests/simple_thread_pool_tests.cpp) - tests for the simple pool
- [advanced_thread_pool](advanced_thread_pool) - advanced thread-pool implementation
- [advanced_thread_pool/include/advanced_thread_pool.h](advanced_thread_pool/include/advanced_thread_pool.h) - public header for the advanced pool
- [advanced_thread_pool/src/advanced_thread_pool.cpp](advanced_thread_pool/src/advanced_thread_pool.cpp) - implementation of the advanced pool
- [advanced_thread_pool/src/task_queue.cpp](advanced_thread_pool/src/task_queue.cpp) - internal queue implementation used by the advanced pool
- [advanced_thread_pool/tests/advanced_thread_pool_tests.cpp](advanced_thread_pool/tests/advanced_thread_pool_tests.cpp) - tests for the advanced pool

## How it works

The simple pool uses a shared task queue and a set of worker threads.
When `do_work` is called, the task is placed in the queue and a worker thread executes it.
The advanced pool uses several internal task queues and distributes work across them to reduce contention.
In both cases, the pool is designed to shut down cleanly and to behave predictably under normal use.

## Build and run

From the project root, configure and build the project with:

```bash
cmake -S . -B build
cmake --build build
```

To run the example program:

```bash
./build/thread_pool_tester
```

On Windows, the executable may be named `thread_pool_tester.exe` depending on your generator.

## Tests

The project includes tests for both implementations.
You can run the full test suite with:

```bash
ctest --test-dir build -VV
```

You can also run the individual test binaries directly from the build tree if needed.

## Notes

This repository is primarily a learning project, but it is also written and structured in a way that reflects careful engineering habits and can be a useful reference for future work.
