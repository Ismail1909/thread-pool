#include "advanced_thread_pool.h"

#include <stdexcept>
#include <mutex>
#include <iostream>

std::mutex cout_guard;

inline void advanced_thread_pool::stop() {
    m_stopped = true;
    for(auto& thread : m_threads) {
        if(thread.joinable()) {
            thread.join();
        }
    }
}

advanced_thread_pool::advanced_thread_pool(std::size_t count) :
 m_task_queue{count}, m_no_of_threads{count}, m_stopped{false}, m_current_index{0} {
    if(count == 0) throw std::invalid_argument("Number of threads must be > 0"); 

    // Init threads
    m_threads.reserve(count);

    try {
        for(std::size_t i = 0 ; i < m_no_of_threads ; ++i) {
            m_threads.emplace_back([this, index = i](){
                while(true) {

                    auto work = std::move(m_task_queue.try_get(index));
                    if(!work) {
                        for(std::size_t i = 0 ; i < m_no_of_threads ; ++i) {
                            work = std::move(m_task_queue.try_get(i));
                            if(work) break;
                        }
                    }

                    try {
                            if(work) {
                                (*work)();
                            }
                            else {
                                if(m_stopped) break;
                            }
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
        };
    } 
    catch(const std::system_error& ex) {
        stop();       
        throw;
    }
}

advanced_thread_pool::~advanced_thread_pool() {
    stop();
}

auto advanced_thread_pool::do_work(work_item_t work_item) -> bool {
    m_current_index = (m_current_index + 1) % m_no_of_threads;
    return m_task_queue.try_add(m_current_index, work_item);
}
