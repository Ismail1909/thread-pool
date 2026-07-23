# Benchmarking

This document describes the setup, methodology, and results of the benchmarking performed on the thread pool implementations against a well known pool like the Grand Central Dispatch (GCD) library. The goal of the benchmarking is to compare the performance of the thread pool implementations in terms of several metrics such as scheduling and synchronization overhead, CPU bound tasks, and memory bound tasks.

## Environment Setup

- Clang compiler (version 18.0 or higher)

- WSL (Windows Subsystem for Linux) with Ubuntu 20.04 or higher (or Ubuntu 20.04 or higher).

## Build GCD Dispatch Static Library.

1. Clone the `swift-corelibs-libdispatch` repository from Github.

    ```bash
    git clone https://github.com/swiftlang/swift-corelibs-libdispatch.git

    cd swift-corelibs-libdispatch
    ```

2. Install the required packages.

    ```bash
    sudo apt-get update

    sudo apt-get install cmake ninja-build clang systemtap-sdt-dev libbsd-dev linux-libc-dev
    ```

3. Setup the build environment.

    ```bash
    mkdir build-static 
    cd build-static

    cmake .. -G Ninja -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DBUILD_SHARED_LIBS=OFF
    ```

    >Note: The `-DBUILD_SHARED_LIBS=OFF` flag is used to build the static library instead of the shared library.

4. Build the static library & install into the usr shared directory.

    ```bash
    ninja
    sudo ninja install
    ```

    Output:

    ```
    [0/1] Install the project...
    -- Install configuration: ""
    --  /usr/local/include/dispatch/base.h
    --  /usr/local/include/dispatch/block.h
    --  /usr/local/include/dispatch/data.h
    --  /usr/local/include/dispatch/dispatch.h
    --  /usr/local/include/dispatch/group.h
    --  /usr/local/include/dispatch/introspection.h
    --  /usr/local/include/dispatch/io.h
    --  /usr/local/include/dispatch/object.h
    --  /usr/local/include/dispatch/once.h
    --  /usr/local/include/dispatch/queue.h
    --  /usr/local/include/dispatch/semaphore.h
    --  /usr/local/include/dispatch/source.h
    --  /usr/local/include/dispatch/time.h
    --  /usr/local/share/man/man3/dispatch.3
    --  /usr/local/share/man/man3/dispatch_after.3
    --  /usr/local/share/man/man3/dispatch_api.3
    --  /usr/local/share/man/man3/dispatch_apply.3
    --  /usr/local/share/man/man3/dispatch_async.3
    --  /usr/local/share/man/man3/dispatch_data_create.3
    --  /usr/local/share/man/man3/dispatch_group_create.3
    --  /usr/local/share/man/man3/dispatch_io_create.3
    --  /usr/local/share/man/man3/dispatch_io_read.3
    --  /usr/local/share/man/man3/dispatch_object.3
    --  /usr/local/share/man/man3/dispatch_once.3
    --  /usr/local/share/man/man3/dispatch_queue_create.3
    --  /usr/local/share/man/man3/dispatch_read.3
    --  /usr/local/share/man/man3/dispatch_semaphore_create.3
    --  /usr/local/share/man/man3/dispatch_source_create.3
    --  /usr/local/share/man/man3/dispatch_time.3
    --  /usr/local/include/os/generic_base.h
    --  /usr/local/include/os/generic_unix_base.h
    --  /usr/local/include/os/generic_win_base.h
    --  /usr/local/include/os/object.h
    --  /usr/local/include/Block.h
    --  /usr/local/lib/libBlocksRuntime.a
    --  /usr/local/lib/libdispatch.a
    ```

5. Link GCD to the benchmark project.

    Add the following to the `CMakeLists.txt` file:

    ```cmake
    # Add swift's gcd libdispatch if the build is Clang & in unix platform.
    if(UNIX AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_link_directories(thread_pool_benchmark PRIVATE /usr/local/lib)

    set(GCD dispatch BlocksRuntime)

    target_compile_options(thread_pool_benchmark PRIVATE -fblocks)

    else()

    set(GCD )

    endif()

    target_link_libraries(
        thread_pool_benchmark
        PRIVATE
        simple_thread_pool
        advanced_thread_pool
        ${GCD} # Link GCD Here.
    )

    ```

    > **Note:** `libdispatch` uses Clang blocks (`^{ ... }`) in many of its APIs. For example:

    ```cpp
    dispatch_group_async(group, queue, ^{
        noop();
    });
    ```

    To compile code that uses block literals, add the `-fblocks` compiler option and link against the `BlocksRuntime` library.

## Benchmarking Methodology

### Benchmarking Timer 

We created a RAII timer class to measure the duration of each benchmark. the timer captures the start point in its constructor and the end point its destructor then the duration is calculated and printed to the console.

The timer class uses `std::chrono::steady_clock` as it's the most reliable clock for measuring intervals because it is not affected by system clock changes or NTP adjustments. The goal is to measure time intervals so we don't care about the actual irl time. The complete implementation is in `benchmark/timer.h`

### Benchmarking Metrics

The libraries are built in release mode with optimizations enabled `-O3` and the benchmarks are run on the system's supported number of threads and `1,000,000` submitted tasks. The following metrics are measured:

1. Pool setup, scheduling & teardown overhead (noop benchmark)

    We introduce a no operation function that does nothing and measure the time it takes to schedule and execute a large number of these no operation tasks. This metric gives us an idea of the overhead introduced by the thread pool's setup, scheduling and teardown mechanisms.

    Example:

    ```cpp
    void noop() {
        // Do nothing.
    }

    void advanced_pool_noop() {
        Timer timer("advanced_pool_noop");

        advanced_thread_pool pool{no_of_threads};
        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            pool.do_work([]() {
                noop();
            });
        }
    }
    ```

2. CPU bound tasks (cpu_bound benchmark)

    We introduce fibonacci function as a cpu bound task that only uses the stack & performs CPU calculations with no memory access and introduce some variablility in the input numbers so the tasks take different amount of time to complete. This metric gives us an idea of how well the thread pool can handle CPU bound tasks without memory contention overhead.

    Example:

    ```cpp

    constexpr uint8_t PATTERN_SIZE = 8;
    std::array<uint32_t, PATTERN_SIZE> pattern {64, 68, 72, 76, 80, 84, 88, 92};

    uint64_t fibonacci(uint64_t n) {
        int a = 0 , b = 1;
        while(n--) {
            auto temp = a + b;
            a = b;
            b = temp;
        }

        return a;
    }

    void advanced_pool_cpu_bound() {
        Timer timer("advanced_pool_cpu_bound");

        advanced_thread_pool pool{no_of_threads};
        for(uint64_t i = 0 ; i < NUMBER_OF_TASKS ; ++i) {
            const auto n = pattern[i % PATTERN_SIZE];
            pool.do_work([n]() {
                auto result = fibonacci(n);
                do_not_optimize(result);
            });
        }
    }
    ```

3. Memory bound tasks (memory_bound benchmark)

    We introduce a memory bound task that reads from a large shared arrray residing on RAM, the tasks just read small chunks from the array and do simple sum, and due to the large volume of tasks, they will be competing for memory locations, this metric gives us an idea of how well the thread pools can handle memory bound tasks with high contention.

    ```cpp

    constexpr uint64_t DATA_SIZE = (512*1024*1024) / sizeof(uint64_t);
    std::array<uint64_t, DATA_SIZE> data{1};

    constexpr uint64_t CHUNK_SIZE = (1*1024*1024) / sizeof(uint64_t); // 1 MB

    inline uint64_t array_sum(uint64_t begin, uint64_t end) {
        uint64_t sum = 0;
        for (uint64_t j = begin; j < end; ++j)
            sum += data[j];

        return sum;
    }

    void advanced_pool_memory_bound() {
        Timer timer("advanced_pool_memory_bound");
        
        advanced_thread_pool pool{no_of_threads};
        for (uint64_t i = 0; i < NUMBER_OF_TASKS; ++i)
        {
            const uint64_t begin = i % (DATA_SIZE - CHUNK_SIZE);

            pool.do_work([&, begin]
            {
                uint64_t sum = array_sum(begin, begin + CHUNK_SIZE);
                do_not_optimize(sum);
            });
        }
    }

    ```

### Note on `do_not_optimize`

```cpp
// Prevents compiler from optimizing away the variable with no overhead.
// This works with GCC & CLANG.
template<typename T>
inline void do_not_optimize(const T& value) {
    asm volatile("" : : "r,m"(value) : "memory");
}
```

This helper originates from the Google Benchmark library. Its purpose is to convince the compiler that `value` is still required after the computation, preventing it from eliminating either the variable or the computation that produced it. Without such a barrier, an optimizing compiler may determine that the result is never used and remove the entire calculation, leading to misleading benchmark results.

The implementation uses GCC/Clang inline assembly. Although the assembly instruction itself does nothing, it creates an optimization barrier that forces the compiler to treat `value` as being observed.

The components of the inline assembly are:

- **`asm volatile`** — Declares an inline assembly statement. The `volatile` qualifier tells the compiler that the statement has important side effects and must not be removed or reordered.

- **`""`** — The assembly instruction. It is intentionally empty because no machine instruction needs to be executed; the statement exists solely as a compiler barrier.

- **First `:`** — The output operand list. It is empty because the assembly does not produce any outputs.

- **`"r,m"(value)`** — The input operand list. It tells the compiler that `value` is consumed by the assembly statement. The `"r,m"` constraint allows the compiler to provide `value` either in a register (`r`) or directly from memory (`m`).

- **`"memory"`** — The clobber list. This informs the compiler that the assembly may access or modify memory, preventing it from making assumptions about memory contents across the statement. This acts as a memory barrier, ensuring that surrounding memory operations are not optimized across the inline assembly.

## Benchmarking Results

Tested on `Intel(R) Core(TM) i5-4460  CPU @ 3.20GHz   3.20 GHz` with `4` threads.

```
System's no of threads: 4
## Scheduling & Synchronization Overhead ##
simple_pool_noop duration: 32224677 microsecs (32224.7 ms)
advanced_pool_noop duration: 15447055 microsecs (15447.1 ms)
gcd_dispatch_noop duration: 685656 microsecs (685.656 ms)
## Small CPU Bound Task ##
simple_pool_cpu_bound duration: 24983726 microsecs (24983.7 ms)
advanced_pool_cpu_bound duration: 14117910 microsecs (14117.9 ms)
gcd_dispatch_cpu_bound duration: 1239796 microsecs (1239.8 ms)
## Memory Bound Task ##
simple_pool_memory_bound duration: 30873566 microsecs (30873.6 ms)
advanced_pool_memory_bound duration: 28259462 microsecs (28259.5 ms)
gcd_dispatch_memory_bound duration: 28414654 microsecs (28414.7 ms)
## END BENCHMARK ##
```

## Notes on Results

The results obviously show the GCD library superiority. Aside from that, the advanced thread pool is significantly faster than the simple thread pool as the advanced one has multiple queues, task stealing which reduces contention and synchronization overhead. The simple thread pool has a single queue and uses a mutex to synchronize access to the queue, which introduces contention and overhead when multiple threads try to access the queue simultaneously.

The results are expected as the logic dictates except for the memory bound task where the performances of the three libraries is very close, I don't have a clear explanation for that, I suspect maybe the methodology is not logically valid enough, I need to rethink the method to confirm the results.

I need to run the benchmark on other devices and think about other metrics later on.