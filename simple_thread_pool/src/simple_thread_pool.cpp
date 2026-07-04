#include "simple_thread_pool.h"

#include <chrono>
#include<iostream>

static std::mutex cout_guard;

void dummy_work() {
    using namespace std::chrono_literals;

    std::unique_lock<std::mutex> lck{cout_guard};

    std::cout << std::this_thread::get_id() <<  " thread working...." << std::endl;
    std::this_thread::sleep_for(2s);
    std::cout << std::this_thread::get_id() <<  " thread done...." << std::endl;
}

void simple_thread_pool::stop() {
    // lock the queue 
    // add a null work item as a signal to end threads 
    // unlock then notify
    {
        std::unique_lock<std::mutex> lck{m_lock_queue};
        m_stopped = true;
        m_work_queue.push(work_item_ptr_t{nullptr});
    }
    m_cv.notify_all();
}

simple_thread_pool::simple_thread_pool(std::size_t count) : m_stopped{false}
{
    if(count == 0) throw std::invalid_argument("Number of threads must be > 0"); 

    // Init threads
    m_threads.reserve(count);

    for(std::size_t i = 0 ; i < count ; ++i) {
        try {
            m_threads.emplace_back([this]() {
                    // lock queue
                    // wait on the conditional variable
                    // inspect queue front
                    // if null work -> break and exit
                    // else pop , unlock & do work
                    
                    while(true) {
                        work_item_ptr_t work{nullptr};
                        {
                            std::unique_lock<std::mutex> lck{m_lock_queue}; // mutex locked
                            m_cv.wait(lck, [&work_queue = m_work_queue] {  // mutex unlocked until the function returns
                                return !work_queue.empty();
                            });
                            work =  std::move(m_work_queue.front()); // check the work item
                            if(!work) break; // end the thread if the work is null which signals the end of the pool.
                            m_work_queue.pop();
                        }
                        // do work
                        try {
                            (*work)();
                        }
                        catch(const std::exception& ex) {
                            std::unique_lock cerr_lock{cout_guard};
                            std::cerr << "Task threw exception: " << ex.what() << std::endl;
                        }
                        catch(...) {
                            std::unique_lock cerr_lock{cout_guard};
                            std::cerr << "Task threw unknown exception" << std::endl;
                        }
                    }
            });
        }
        catch(const std::system_error& ex) {
            stop();
            for(auto& thread : m_threads) {
                if(thread.joinable()) {
                    thread.join();
                }
            }
            throw;
        }
    }
}

simple_thread_pool::~simple_thread_pool() {
    stop();

    // wait for all threads to finish and exit gracefully.
    for(auto& thread : m_threads) {
        thread.join();
    }
}

void simple_thread_pool::do_work(work_item_t work_item)
{
    // Reject null function
    if(!work_item) {
        throw std::invalid_argument{"Function cannot be null"};
    }

    // Create work unique pointer.
    work_item_ptr_t work_item_ptr = std::make_unique<work_item_t>(work_item);
    // Lock queue and push work item to it.
    {
        std::unique_lock<std::mutex> lck{m_lock_queue};
        // Don't accept new work if the thread pool is stopped.
        if(m_stopped) {
            return;
        }
        m_work_queue.push(std::move(work_item_ptr));
    }
    // Unlock queue

    // notify one waiting thread using the condition variable.
    m_cv.notify_one();
}
