#include <iostream>
#include <thread>
#include <list>
#include <mutex>
#include <algorithm>
#include <deque>

#include <memory>
#include <exception>
#include <stack>

namespace DataSharing{

    class background_task{
    public:
        void operator()() const{
            std::cout << "Hello from function object" << std::endl;
        }

    };

}

namespace MutexExample{
    /*
     * Guideline: don't pass pointers or references to protected data outside the scope of the lock
     * whether by
     *  Returning them from a function
     *  Storing them in externally visible memory
     *  Passing them as arguments to user supplied functions
     */
    std::list<int> int_list;
    std::mutex int_list_mutex;

    void add_to_list(int new_value){
        std::lock_guard guard(int_list_mutex); // cpp17, class template argument deduction
        int_list.push_back(new_value);
    }

    bool list_contains(int value_to_find){
        std::scoped_lock guard(int_list_mutex);
        return std::find(int_list.begin(), int_list.end(), value_to_find) != int_list.end();
    }

    void add_some_ints(){
        for(int i = 5; i < 20; i++){
            add_to_list(i);
        }
    }

    void check_for_some_ints(){
        for(int i = 3; i < 15; i+=2){
            std::cout << i << " " << list_contains(i) << std::endl;
        }
    }


}

namespace AdaptedStack{

    struct empty_stack: std::exception {
        [[nodiscard]] const char* what() const override {throw("empty");}
    };

    template<typename T>
    class threadsafe_stack{
    private:
        std::stack<T> data;
        mutable std::mutex m;
    public:
        threadsafe_stack() = default;
        threadsafe_stack(const threadsafe_stack& other){
            std::lock_guard<std::mutex> lock(other.m);
            data = other.data;
        }
        threadsafe_stack& operator=(const threadsafe_stack&) = delete;
        void push(T new_value){
            std::lock_guard<std::mutex> lock(m);
            data.push(std::move(new_value));
        }
        std::shared_ptr<T> pop(){
            std::lock_guard<std::mutex> lock(m);
            if(data.empty()){
                throw empty_stack();
            }
            std::shared_ptr<T> const res(std::make_shared<T>(data.top()));
            data.pop();
            return res;
        }
        void pop(T& value){
            std::lock_guard<std::mutex> lock(m);
            if(data.empty()){
                throw empty_stack();
            }
            value = data.top();
            data.pop();
        }
        bool empty() const{
            std::lock_guard<std::mutex> lock(m);
            return data.empty();
        }

    };
}
