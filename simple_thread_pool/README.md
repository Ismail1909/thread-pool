# simple_thread_pool

This directory contains a small and simple C++ thread pool implementation.
It is implemented using only the C++ standard library and has no external dependencies.
It is designed as a learning example and keeps the API minimal.

## What it provides

The library exposes one class:

- `simple_thread_pool`

You can create a pool with a chosen number of worker threads and submit work items using `do_work`.

## Basic usage

```cpp
#include "simple_thread_pool.h"

int main() {
    simple_thread_pool pool{4};

    pool.do_work([] {
        // work to run in a background thread
    });
}
```

## Behavior

- The constructor throws `std::invalid_argument` if the thread count is zero.
- `do_work` throws `std::invalid_argument` if you pass a null function.
- The destructor stops the workers safely and waits for them to finish.

## Internal design

The implementation uses:

- `std::thread` for worker threads
- `std::mutex` to protect the task queue
- `std::condition_variable` to wake workers when new work arrives

## Logical flow

1. A thread pool is created with a specified number of worker threads.
2. Work items are submitted to the pool using `do_work`.
3. Each work item is added to a task queue with is shared among the worker threads.
4. Worker threads continuously check the task queue for new work.
5. When new work is available, a worker thread picks it up and executes it.
6. The destructor ensures all workers finish their current tasks before stopping.

## Limitations
- Since the thread share the same task queue, that leads to contention & contention leads to more synchroinzation overhead which degrades performance.

## Files

- [include/simple_thread_pool.h](include/simple_thread_pool.h) - class declaration
- [src/simple_thread_pool.cpp](src/simple_thread_pool.cpp) - implementation
- [tests/simple_thread_pool_tests.cpp](tests/simple_thread_pool_tests.cpp) - simple tests

## Testing

