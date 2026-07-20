# advanced_thread_pool

This directory contains an advanced C++ thread pool implementation.
It is implemented using only the C++ standard library and has no external dependencies.
It is designed as a learning example and explores a more advanced scheduling approach with multiple task queues.

## What it provides

The library exposes one class:

- `advanced_thread_pool`

You can create a pool with a chosen number of worker threads and submit work items using `do_work`.

## Basic usage

```cpp
#include "advanced_thread_pool.h"

int main() {
    advanced_thread_pool pool{4};

    pool.do_work([] {
        // work to run in a background thread
    });
}
```

## Behavior

- The constructor throws `std::invalid_argument` if the thread count is zero.
- `do_work` throws `std::invalid_argument` if you pass a null function.
- The destructor stops the workers safely and waits for them to finish.
- Tasks are distributed across multiple internal task queues so that worker threads can pick up work from different queues.

## Internal design

The implementation uses:

- `std::thread` for worker threads
- `std::vector<task_queue>` to hold multiple task queues
- `std::atomic` to track the next queue for task distribution
- `std::mutex` and `std::condition_variable` in the task queue implementation to coordinate workers

## Logical flow

1. A thread pool is created with a specified number of worker threads.
2. Each worker thread is assigned its own queue and the pool maintains several task queues internally.
3. Work items are submitted to the pool using `do_work`.
4. New work is distributed across the available queues to reduce contention.
5. Worker threads check the queues for new work and execute it when available.
6. The destructor ensures all workers finish their current tasks before stopping.

## Limitations

- Work is distributed heuristically rather than by a sophisticated load-balancing strategy.

## Files

- [include/advanced_thread_pool.h](include/advanced_thread_pool.h) - class declaration
- [src/advanced_thread_pool.cpp](src/advanced_thread_pool.cpp) - implementation
- [src/task_queue.cpp](src/task_queue.cpp) - queue implementation used by the pool
- [tests/advanced_thread_pool_tests.cpp](tests/advanced_thread_pool_tests.cpp) - tests

## Testing
