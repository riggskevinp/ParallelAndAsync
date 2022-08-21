#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>

#include <random>
#include <chrono>

struct data_chunk{
    std::time_t time{0};
    double position{0};
    double orientation{0};
    data_chunk() = default;
    data_chunk(std::time_t _time, double _position, double _orientation):
    time(_time), position(_position), orientation(_orientation){}
};

void process(data_chunk d){
    std::cout << "time: " << d.time << std::endl;
    std::cout << "position: " << d.position << std::endl;
    std::cout << "orientation: " << d.orientation << std::endl << std::endl;
};

data_chunk prepare_data(){
    std::random_device rand_dev;
    std::mt19937 gen(rand_dev());
    std::uniform_real_distribution<> uni_dist(0.0,100.0);
    return {std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()),uni_dist(gen),uni_dist(gen)};
}

namespace ConditionVariables{

    int number_of_chunks = 5;
    int number_of_chunks_prepared = 0;
    bool more_data_to_prepare(){
        if(number_of_chunks_prepared < number_of_chunks){
            number_of_chunks_prepared += 1;
            return true;
        } else {
            return false;
        }
    }


    std::mutex mut;
    std::queue<data_chunk> data_queue;
    std::condition_variable data_cond;

    bool is_last_chunk(){
        if(number_of_chunks_prepared == number_of_chunks && data_queue.empty()){
            return true;
        } else {
            return false;
        }
    }

    void data_preparation_thread(){
        while(more_data_to_prepare()){
            data_chunk const data = prepare_data();
            data_queue.push(data);
        }
        data_cond.notify_one();
    }

    void data_processing_thread(){
        while(true){
            std::unique_lock<std::mutex> lk(mut);
            data_cond.wait(lk, []{return !data_queue.empty();});
            data_chunk data=data_queue.front();
            data_queue.pop();
            lk.unlock();

            process(data);
            if(is_last_chunk()){
                break;
            }
        }
    }

} // ConditionVariables


namespace ThreadSafe_Queue_ConditionVariables{

    // std::queue interface
    /*
    template<class T, class  Container = std::deque<T>>
    class queue{
    public:
        explicit queue(const Container&);
        explicit queue(Container&& = Container());
        template<class Alloc> explicit queue(const Alloc&);
        template<class Alloc> queue(const Container&, const Alloc&);
        template<class Alloc> queue(Container&&, const Alloc&);
        template<class Alloc> queue(queue&&, const Alloc&);
        void swap(queue& q);
        bool empty() const;
        size_t size() const;
        T& front();
        const T& front() const;
        T& back();
        const T& back() const;
        void push(const T& x);
        void push(T&& x);
        void pop();
        template<class... Args> void emplace(Args&&... args);
    };*/

    // Thread safe queue interface
    template<typename T>
    class threadsafe_queue{
    private:
        mutable std::mutex mut;
        std::queue<T> data_queue;
        std::condition_variable data_cond;
    public:
        threadsafe_queue() = default;
        threadsafe_queue(const threadsafe_queue& other){
            std::lock_guard<std::mutex> lk(other.mut);
            data_queue= other.data_queue;
        }
        //threadsafe_queue& operator=(const threadsafe_queue&) = delete;
        void push(T new_value) {
            std::lock_guard<std::mutex> lk(mut);
            data_queue.push(new_value);
            data_cond.notify_one();
        }
        bool try_pop(T& value){
            std::lock_guard<std::mutex> lk(mut);
            if(data_queue.empty()){
                return false;
            }
            value = data_queue.front();
            data_queue.pop();
            return true;
        }
        std::shared_ptr<T> try_pop(){
            std::lock_guard<std::mutex> lk(mut);
            if(data_queue.empty()){
                return std::shared_ptr<T>();
            }
            std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
            data_queue.pop();
            return res;
        }
        void wait_and_pop(T& value) {
            std::unique_lock<std::mutex> lk(mut);
            data_cond.wait(lk, [this] {return !data_queue.empty();});
            value = data_queue.front();
            data_queue.pop();
        }
        std::shared_ptr<T> wait_and_pop(){
            std::unique_lock<std::mutex> lk(mut);
            data_cond.wait(lk, [this]{return !data_queue.empty();});
            std::shared_ptr<T> res(std::make_shared<T>(data_queue.front()));
            data_queue.pop();
            return res;
        }
        [[nodiscard]] bool empty() const{
            std::lock_guard<std::mutex> lk(mut);
            return data_queue.empty();
        }
    };

    threadsafe_queue<data_chunk> data_queue;

    int number_of_chunks = 5;
    int number_of_chunks_prepared = 0;
    bool more_data_to_prepare(){
        if(number_of_chunks_prepared < number_of_chunks){
            number_of_chunks_prepared += 1;
            return true;
        } else {
            return false;
        }
    }
    bool is_last_chunk(){
        if(number_of_chunks_prepared == number_of_chunks && data_queue.empty()){
            return true;
        } else {
            return false;
        }
    }

    void data_preparation_thread(){
        while(more_data_to_prepare()){
            data_chunk const data = prepare_data();
            data_queue.push(data);
        }
    }

    void data_processing_thread(){
        while(true){
            data_chunk data;
            data_queue.wait_and_pop(data);
            process(data);
            if(is_last_chunk()){
                break;
            }
        }
    }

} // ThreadSafe_Queue_ConditionVariables