#include <iostream>
#include <thread>

#include "basics.hpp"

void hello(){
    std::cout << "Hello most basic thread" << std::endl;
}

void hello_string(const std::string& s){
    std::cout << "Hello from " << s << std::endl;
}

int main(){
    std::thread t(hello);
    t.join();

    // Basic threads for function object
    basics::background_task bg_task;
    std::thread bg_thread(bg_task);
    std::thread bg_thread2{basics::background_task()};// uniform initialization syntax
    bg_thread.join();
    bg_thread2.join();

    // basic lambda
    std::thread lambda_thread([]{
        std::cout << "Hello from lambda" << std::endl;
    });
    lambda_thread.join();


    // Memory access concerns
    memory_management::guarded_thread();

    // Thread ownership
    thread_ownership::generate();
    thread_ownership::scoped_thread(std::thread(hello_string, "scoped"));

    // Joinable thread
    thread_ownership::joining_thread(hello_string, "Joining Thread");

    // Vector of work threads
    thread_ownership::vector_of_work_threads();

    // Threads at runtime
    // Parallel accumulate
    std::vector<int> v{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for(int i = 10; i < 100; ++i){
        v.emplace_back(i);
    }
    int initial = 0;
    auto res = threads_at_runtime::parallel_accumulate(v.begin(), v.end(), initial);
    std::cout << "accumulated value: " << res << std::endl;


}