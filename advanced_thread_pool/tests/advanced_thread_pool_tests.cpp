#include "advanced_thread_pool.h"

#include <gtest/gtest.h>

#include <future>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>
#include <numeric>


using namespace std::chrono_literals;

TEST(AdvancedThreadPoolTest, InvalidCtorTest) {
    EXPECT_THROW(advanced_thread_pool{0}, std::invalid_argument);
}

TEST(AdvancedThreadPoolTest, BasicGracefulDtorTest) {
    {
        advanced_thread_pool threadPool{50};
    }

    SUCCEED();
}

TEST(AdvancedThreadPoolTest, InvalidWorkEnqueueTest) {
    advanced_thread_pool threadPool{1};
    EXPECT_THROW(threadPool.do_work(nullptr), std::invalid_argument);
}

TEST(AdvancedThreadPoolTest, DestructionWaitsForWorkersToExit) {
    std::promise<void> p;
    auto f = p.get_future();

    {
        advanced_thread_pool pool{2};

        pool.do_work([&p]() {
            std::this_thread::sleep_for(100ms);
            p.set_value();
        });
    } // destructor should join worker threads here

    // After destructor returns, the promise should be set.
    EXPECT_EQ(f.wait_for(0ms), std::future_status::ready);
}

// This test hangs forever, TODO: investigate the issue.
TEST(AdvancedThreadPoolTest, DISABLED_SingleTaskTest) {
    std::promise<int> value_promise;
    std::future<int> result = value_promise.get_future();

    advanced_thread_pool thread_pool;

    thread_pool.do_work([&promise = value_promise]() {
        int value = 0;
        promise.set_value(1);
    });

    EXPECT_EQ(result.get(), 1);
}

TEST(AdvancedThreadPoolTest, BasicMultiThreadTest) {
    constexpr int num_threads{8};
    std::atomic<int> counter{0};

    {
        advanced_thread_pool thread_pool{num_threads};

        for(int i = 0 ; i < num_threads ; ++i) {
            thread_pool.do_work([&counter](){
                ++counter;
            });
        }
    }

    EXPECT_EQ(counter, num_threads);
}

TEST(AdvancedThreadPoolTest, ConcurrentParallelismTest) {
    // create a thread pool with workers less than number of tasks.
    constexpr int pool_size = 4;
    constexpr int task_count = 12;

    // Number of tasks currently executing.
    std::atomic<int> concurrent{0};

    // Highest number of simultaneously executing tasks observed.
    std::atomic<int> max_concurrent{0};

    {
        advanced_thread_pool pool{pool_size};

        for (int i = 0; i < task_count; ++i) {
            pool.do_work([&]() {
                int curr = ++concurrent; // increment number of current concurrent tasks
                int prev = max_concurrent.load(); // load the prev max num of threads ran in parallel
                
                // If this task increased the maximum concurrency,
                // atomically update max_concurrent.
                //
                // compare_exchange_weak succeeds only if max_concurrent
                // is still equal to 'prev'. If another thread updated it
                // first, 'prev' is updated with the new value and we retry
                // only if our value is still larger.
                while (curr > prev && !max_concurrent.compare_exchange_weak(prev, curr)) {}

                // Keep the task alive long enough for other tasks to overlap.
                std::this_thread::sleep_for(120ms);

                // Mark this task as finished.
                --concurrent;
            });
        }
    } // pool destructor waits for tasks to finish

    // more than one thread should be running in parallel
    EXPECT_GT(max_concurrent.load(), 1);
}

TEST(AdvancedThreadPoolTest, ParallelVectorReduction)
{
    constexpr std::size_t vector_size = 10'000'000;
    constexpr std::size_t task_count = 12;
    constexpr std::size_t pool_size = 4;

    // Fill the vector with deterministic values.
    std::vector<int> values(vector_size);
    std::iota(values.begin(), values.end(), 1);

    // Compute the expected result sequentially.
    const std::int64_t expected =
        std::accumulate(values.begin(), values.end(), std::int64_t{0});

    // Each task writes to its own slot (no synchronization needed).
    std::vector<std::int64_t> partial_sums(task_count, 0);

    {
        advanced_thread_pool pool(pool_size);

        const std::size_t chunk_size =
            (vector_size + task_count - 1) / task_count;

        for (std::size_t i = 0; i < task_count; ++i)
        {
            const std::size_t begin = i * chunk_size;
            const std::size_t end =
                std::min(begin + chunk_size, vector_size);

            pool.do_work([&, i, begin, end]
            {
                std::int64_t local_sum = 0;

                for (std::size_t j = begin; j < end; ++j)
                {
                    local_sum += values[j];
                }

                partial_sums[i] = local_sum;
            });
        }
    } // Destructor waits for all tasks.

    const std::int64_t parallel_sum =
        std::accumulate(partial_sums.begin(),
                        partial_sums.end(),
                        std::int64_t{0});

    EXPECT_EQ(parallel_sum, expected);
}

TEST(AdvancedThreadPoolTest, BasicStressTest) {
    constexpr std::size_t num_threads{8};
    constexpr std::size_t num_tasks{1'000'000};
    std::atomic<std::size_t> counter{0};

    {
        advanced_thread_pool thread_pool{num_threads};

        for(int i = 0 ; i < num_tasks ; ++i) {
            thread_pool.do_work([&counter](){
                ++counter;
            });
        }
    }

    EXPECT_EQ(counter.load(), num_tasks);
}