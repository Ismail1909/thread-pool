# Thread Pool

This project is a small C++ example that shows how to build a simple thread pool.
It is implemented using only the C++ standard library, so there are no external dependencies.
It is meant to be easy to read and understand, so it focuses on the core ideas:

- create a pool of worker threads
- submit tasks to it
- let those tasks run in the background
- shut the pool down cleanly when the program finishes

## Project structure

- [main.cpp](main.cpp) - a small example program that creates a pool and submits work
- [simple_thread_pool](simple_thread_pool) - the actual thread pool library
- [simple_thread_pool/include/simple_thread_pool.h](simple_thread_pool/include/simple_thread_pool.h) - public header
- [simple_thread_pool/src/simple_thread_pool.cpp](simple_thread_pool/src/simple_thread_pool.cpp) - implementation
- [simple_thread_pool/tests/simple_thread_pool_tests.cpp](simple_thread_pool/tests/simple_thread_pool_tests.cpp) - basic tests

## How it works

The library uses a queue of tasks plus a set of worker threads.
When you call `do_work`, the task is added to the queue.
A worker thread picks it up and runs it.
When the pool is destroyed, it signals the workers to stop and waits for them to finish.

## Build and run

From the project root, run:

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

You can run the test suite with:

```bash
ctest -VV
```

## Notes

This is an educational implementation, as I progres I will add more complexities & challenges.
