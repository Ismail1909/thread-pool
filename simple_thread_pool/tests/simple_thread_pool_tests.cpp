#include "simple_thread_pool.h"

#include <gtest/gtest.h>

#include <future>

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
