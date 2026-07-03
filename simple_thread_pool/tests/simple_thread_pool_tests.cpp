#include "simple_thread_pool.h"

#include <gtest/gtest.h>

TEST(SimpleThreadPoolTest, InvalidArgumentTest) {
    EXPECT_THROW(simple_thread_pool{0}, std::invalid_argument);

    simple_thread_pool threadPool{2};
    EXPECT_THROW(threadPool.do_work(nullptr), std::invalid_argument);
}
