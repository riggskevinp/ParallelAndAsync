#include <iostream>
#include <memory>
#include <thread>
#include <list>
#include <mutex>
#include <algorithm>
#include <deque>

#include <exception>
#include <stack>
#include <map>
#include <string>
#include <shared_mutex>
#include <utility>

namespace DataSharing{

    class background_task{
    public:
        void operator()() const{
            std::cout << "Hello from function object" << std::endl;
        }

    };

} // DataSharing

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
        [[nodiscard]] const char* what() const override {return "Stack Empty";}
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

namespace DeadlockProblem{

    class big_object{
        big_object(){}
    };


    void swap(big_object& lhs, big_object& rhs);
    class X{
    private:
        big_object detail;
        std::mutex m;
    public:
        X(big_object const& sd):detail(sd){}
        friend void swap(X& lhs, X& rhs){
            if(&lhs==&rhs){
                return;
            }
            std::scoped_lock guard(lhs.m,rhs.m);
            swap(lhs.detail,rhs.detail);
        }
        friend void std_uniqueLock_lock_swap(X& lhs, X& rhs){
            if(&lhs==&rhs){
                return;
            }
            std::unique_lock<std::mutex>  lock_a(lhs.m,std::defer_lock);
            std::unique_lock<std::mutex>  lock_b(rhs.m,std::defer_lock);
            std::lock(lock_a,lock_b);
            swap(lhs.detail,rhs.detail);
        }
    };

    class Y{
    private:
        int detail;
        mutable std::mutex m;
        int get_detail() const {
            std::lock_guard<std::mutex> lock_a(m);
            return detail;
        }
    public:
        Y(int d):detail(d){}
        friend bool operator==(const Y& lhs, const Y& rhs){
            if(&lhs==&rhs){
                return true;
            }
            int const lhs_value = lhs.get_detail();
            int const rhs_value = rhs.get_detail();
            return lhs_value == rhs_value;
        }
    };
}


namespace SharedDataProtection{
    class Resource{
    public:
        Resource() = default;
        static void say_something(){std::cout << "Resource say something" << std::endl;}
    };

    std::shared_ptr<Resource> resource_ptr;
    std::mutex resource_mutex;
    // thread safe lazy initialization
    void foo(){
        std::unique_lock<std::mutex> lk(resource_mutex);
        if(!resource_ptr){
            resource_ptr = std::make_shared<Resource>();
        }
        lk.unlock();
        resource_ptr->say_something();
    }

    // Thread-safe lazy initialization class member
    /*
    class connection_info;
    class connection_handle;

    class X{
    private:
        connection_info connection_details;
        connection_handle connection;
        std::once_flag connection_init_flag;
        void open_connection(){
            connection = connection_manager.open(connection_details);
        }
    public:
        X(const connection_info& connection_details_):
                connection_details(connection_details_){}
        void send_data(const data_packet & data){
            std::call_once(connection_init_flag, &X::open_connection,this);
            connection.send_data();
        }
        data_packet receive_data(){
            std::call_once(connection_init_flag, &X::open_connection,this);
            return connection.receive_data();
        }
    };*/

    // Data structure with shared mutex
    class Dns_entry{
    private:
        std::string entry;
    public:
        Dns_entry():entry("empty"){}
        explicit Dns_entry(std::string  e):entry(std::move(e)){}
        Dns_entry operator()() const {return *this;}
        std::string to_string() const {return entry;}
    };
    class Dns_cache{
        std::map<std::string, Dns_entry> entries;
        mutable std::shared_mutex entry_mutex;
    public:
        Dns_entry find_entry(const std::string& domain) const{
            std::shared_lock<std::shared_mutex> lk(entry_mutex);
            const std::map<std::string, Dns_entry>::const_iterator it = entries.find(domain); // or const auto
            return (it == entries.end()) ? Dns_entry() : it->second();
        }
        void update_or_add_entry(const std::string& domain, const Dns_entry& dns_details){
            std::lock_guard<std::shared_mutex> lk(entry_mutex);
            entries[domain] = dns_details;
        }
    };
}// SharedDataProtection
