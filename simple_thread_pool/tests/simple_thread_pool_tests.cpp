#include "simple_thread_pool.h"

#include <gtest/gtest.h>

#include <future>
#include <chrono>
#include <atomic>
#include <thread>
#include <vector>


using namespace std::chrono_literals;

TEST(SimpleThreadPoolTest, InvalidCtorTest) {
    EXPECT_THROW(simple_thread_pool{0}, std::invalid_argument);
}

TEST(SimpleThreadPoolTest, BasicGracefulDtorTest) {
    {
        simple_thread_pool threadPool{50};
    }

    SUCCEED();
}

TEST(SimpleThreadPoolTest, InvalidWorkEnqueueTest) {
    simple_thread_pool threadPool{1};
    EXPECT_THROW(threadPool.do_work(nullptr), std::invalid_argument);
}

TEST(SimpleThreadPoolTest, DestructionWaitsForWorkersToExit) {
    std::promise<void> p;
    auto f = p.get_future();

    {
        simple_thread_pool pool{2};

        pool.do_work([&p]() {
            std::this_thread::sleep_for(100ms);
            p.set_value();
        });
    } // destructor should join worker threads here

    // After destructor returns, the promise should be set.
    EXPECT_EQ(f.wait_for(0ms), std::future_status::ready);
}

TEST(SimpleThreadPoolTest, SingleTaskTest) {
    std::promise<int> value_promise;
    std::future<int> result = value_promise.get_future();

    simple_thread_pool thread_pool;

    thread_pool.do_work([&promise = value_promise]() {
        int value = 0;
        promise.set_value(1);
    });

    EXPECT_EQ(result.get(), 1);
}

TEST(SimpleThreadPoolTest, BasicMultiThreadTest) {
    constexpr int num_threads{8};
    std::atomic<int> counter{0};

    {
        simple_thread_pool thread_pool{num_threads};

        for(int i = 0 ; i < num_threads ; ++i) {
            thread_pool.do_work([&counter](){
                ++counter;
            });
        }
    }

    EXPECT_EQ(counter, num_threads);
}

TEST(SimpleThreadPoolTest, ConcurrentParallelismTest) {
    // create a thread pool with workers less than number of tasks.
    constexpr int pool_size = 4;
    constexpr int task_count = 12;

    // Number of tasks currently executing.
    std::atomic<int> concurrent{0};

    // Highest number of simultaneously executing tasks observed.
    std::atomic<int> max_concurrent{0};

    {
        simple_thread_pool pool{pool_size};

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


TEST(SimpleThreadPoolTest, TaskExceptionTest) {

    struct custom_exception {

    };

    {
        simple_thread_pool pool{5};

        // Create task that throws exception
        pool.do_work([](){
            std::this_thread::sleep_for(120ms);

            throw std::runtime_error{"Task Failed"};
        });

        pool.do_work([](){
            std::this_thread::sleep_for(120ms);

            throw custom_exception{};
        });

        for(std::uint32_t i = 0 ; i < 10 ; ++i) {
            pool.do_work([]() {
                std::this_thread::sleep_for(120ms);
            });
        }
    }

    SUCCEED();
}