#include <iostream>
#include <thread>
#include <numeric>

namespace basics{

    class background_task{
    public:
        void operator()() const{
            std::cout << "Hello from function object" << std::endl;
        }

    };

}

namespace memory_management{
    struct func{
        int& i;
        explicit func(int& _i): i(_i){}
        void operator()(){
            for(unsigned j = 0; j < 10; ++j){
                std::cout << j << std::endl;
                std::cout << i << std::endl; // Access to a variable that might be deleted
            }
        }
    };

    void access_after_deleted(){
        int local  = 0;
        func my_func(local);
        std::thread my_thread(my_func);
        my_thread.detach(); // function does not wait for thread to finish
        // my_func may try to access local after deletion; undefined behavior
        // replace with my_thread.join() to ensure no issue
    }

    class thread_guard{
        std::thread& t;
    public:
        explicit thread_guard(std::thread& _t):t(_t){}
        ~thread_guard(){
            if(t.joinable()){
                t.join();
            }
        }
        // copy constructor and copy assignment is potentially dangerous
        // the object could outlive the thread that it is joining
        // marking deleted causes compiler error if attempted
        thread_guard(thread_guard const&)=delete;
        thread_guard& operator=(thread_guard const&)=delete;
    };

    void guarded_thread(){
        int local=0;
        func my_func(local);
        std::thread t(my_func);
        thread_guard g(t);
        std::cout << "Work in current thread" << std::endl;
    }
}

namespace thread_ownership{
    void some_function(){
        std::cout << "Some function" << std::endl;
    }

    void some_other_function(int i){
        std::cout << "Other function " << i << std::endl;
    }

    std::thread return_thread(){
        return std::thread(some_function);
    }

    std::thread return_different_thread(){
        std::thread t(some_other_function, 5);
        return t;
    }

    void accept_thread(std::thread t){
        if(t.joinable()){
            t.join();
        }
    }

    void generate(){
        accept_thread(std::thread(some_function));
        std::thread t(some_function);
        accept_thread(std::move(t));
    }

    // Extend guarded thread to also own the thread it is "guarding"
    class scoped_thread{
        std::thread t;
    public:
        explicit scoped_thread(std::thread t_): t(std::move(t_)){
            if(!t.joinable()){
                throw std::logic_error("No thread");
            }
        }
        ~scoped_thread(){
            t.join();
        }
        scoped_thread(scoped_thread const&)=delete;
        scoped_thread& operator=(scoped_thread const&)=delete;
    };

    // C++ 17 joinable thread proposal implementation
    // C++ 20 jthread
    class joining_thread{
        std::thread t;
    public:
        joining_thread() noexcept=default;

        template<typename Callable, typename ... Args>
        explicit joining_thread(Callable& func, Args&& ... args):
            t(std::forward<Callable>(func), std::forward<Args>(args)...) {}

        explicit joining_thread(std::thread t_) noexcept: t(std::move(t_)){}
        joining_thread(joining_thread&& other) noexcept: t(std::move(other.t)){}
        joining_thread& operator=(joining_thread&& other) noexcept{
            if(joinable()){
                join();
            }
            t=std::move(other.t);
            return *this;
        }
        joining_thread& operator=(std::thread other) noexcept{
            if(joinable()){
                join();
            }
            t=std::move(other);
            return *this;
        }
        ~joining_thread() noexcept{
            if(joinable()){
                join();
            }
        }
        void swap(joining_thread& other) noexcept{
            t.swap(other.t);
        }
        [[nodiscard]] std::thread::id get_id() const noexcept{
            return t.get_id();
        }
        bool joinable(){
            return t.joinable();
        }
        void join(){
            t.join();
        }
        void detach(){
            t.detach();
        }
        std::thread& as_thread() noexcept{
            return t;
        }
        [[nodiscard]] const std::thread& as_thread() const noexcept{
            return t;
        }
    };

    void vector_of_work_threads(){
        std::vector<std::thread> threads;
        for(unsigned i=0; i<20; i++){
            threads.emplace_back(some_other_function, i);
        }
        for(auto& entry: threads){
            entry.join();
        }
    }
}


namespace threads_at_runtime{
    template<typename Iterator, typename T>
    struct accumulate_block{
        void operator() (Iterator first, Iterator last, T& result){
            result = std::accumulate(first, last, result);
        }
    };

    template<typename Iterator, typename T>
    T parallel_accumulate(Iterator first, Iterator last, T init){
        const auto length = (unsigned long)std::distance(first, last);
        if(!length){
            return init;
        }
        unsigned long const min_per_thread = 25;
        unsigned long const max_threads = (length+min_per_thread-1) / min_per_thread;
        unsigned long const hardware_threads = std::thread::hardware_concurrency();
        std::cout << "Hardware threads: " << hardware_threads << std::endl;
        unsigned long const num_threads = std::min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
        std::cout << "Number of threads: " << num_threads << std::endl;
        unsigned long const block_size = length/num_threads;
        std::vector<T> results(num_threads);
        std::vector<std::thread> threads(num_threads-1);
        Iterator block_start = first;
        for(unsigned long i = 0; i < (num_threads - 1); ++i){
            Iterator block_end = block_start;
            std::advance(block_end,block_size);
            threads[i] = std::thread(accumulate_block<Iterator, T>(), block_start, block_end, std::ref(results[i]));
            std::cout << "Thread ID: " << threads[i].get_id() << std::endl;
            block_start = block_end;
        }
        accumulate_block<Iterator,T>()(block_start,last,results[num_threads-1]);
        for(auto& entry: threads){
            entry.join();
        }
        return std::accumulate(results.begin(),results.end(),init);
    }
}